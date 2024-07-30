using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Threading.Tasks;
using Meadow.Hardware;
using Microgpu.Common.Operations;
using Microgpu.Common.Responses;

namespace Microgpu.Common.Comms;

public class MeadowUartGpuCommunication : IGpuCommunication
{
    private const int BufferSize = 2048;

    private readonly ISerialMessagePort _port;
    private readonly PacketFramer _packetFramer = new();
    private readonly byte[] _decodeBuffer = new byte[255];
    private readonly byte[] _encodeBuffer = new byte[255];
    private readonly ConcurrentQueue<byte[]> _readMessages = new();
    private readonly Queue<IFireAndForgetOperation> _operations = new();

    public MeadowUartGpuCommunication(ISerialMessageController serialController, SerialPortName portName)
    {
        if (serialController == null) throw new ArgumentNullException(nameof(serialController));

        _port = serialController.CreateSerialMessagePort(portName, [0x00], true, baudRate: 115200, readBufferSize: BufferSize);
        _port.MessageReceived += PortOnMessageReceived;
        
        _port.Open();
    }

    public ValueTask ResetAsync()
    {
        return new ValueTask();
    }

    public void EnqueueOutboundOperation(IFireAndForgetOperation operation)
    {
        // TODO: Probably should just immediately fire it off 
        _operations.Enqueue(operation);
    }

    public ValueTask SendQueuedOutboundOperationsAsync()
    {
        while (_operations.TryDequeue(out var operationToSend))
        {
            var byteCount = _packetFramer.Encode(operationToSend, _encodeBuffer);
            _port.Write(_encodeBuffer, 0, byteCount);
        }

        return new ValueTask();
    }

    public ValueTask SendImmediateOperationAsync(IOperation operation)
    {
        var byteCount = _packetFramer.Encode(operation, _encodeBuffer);
        _port.Write(_encodeBuffer, 0, byteCount);

        return new ValueTask();
    }

    public async ValueTask<TResponse?> ReadNextResponseAsync<TResponse>() where TResponse : class, IResponse, new()
    {
        // TODO: add timeout
        while (!_readMessages.TryDequeue(out var bytes))
        {
            await Task.Delay(1);
        }

        var result = _packetFramer.Decode(_decodeBuffer);
        if (result.InputBytesProcessed == 0 || result.DecodedBytes.Length == 0)
        {
            // Incomplete or undecodable packet
            return null;
        }

        var response = new TResponse();
        response.Deserialize(result.DecodedBytes.Span);

        return response;
    }

    private void PortOnMessageReceived(object sender, SerialMessageData e)
    {
        _readMessages.Enqueue(e.Message);
    }
}