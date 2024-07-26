using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Meadow.Hardware;
using Microgpu.Common.Operations;
using Microgpu.Common.Responses;

namespace Microgpu.Common.Comms;

public class MeadowSpiGpuCommunication : IGpuCommunication
{
    private readonly IDigitalOutputPort _chipSelectPin;
    private readonly IDigitalInputPort _handshakePin;
    private readonly IDigitalOutputPort _resetPin;
    private readonly ISpiBus _spiBus;
    private readonly Queue<IFireAndForgetOperation> _operations = new();
    private readonly byte[] _buffer = new byte[1024];
    private readonly PacketFramer _packetFramer = new();

    public MeadowSpiGpuCommunication(
        ISpiBus spiBus,
        IDigitalInputPort handshakePin,
        IDigitalOutputPort resetPin,
        IDigitalOutputPort chipSelectPin)
    {
        _spiBus = spiBus;
        _handshakePin = handshakePin;
        _resetPin = resetPin;
        _chipSelectPin = chipSelectPin;
    }

    public async ValueTask ResetAsync()
    {
        // Ensure the chip select starts high/inactive
        _chipSelectPin.State = true;

        // Reset the display
        _resetPin.State = false;
        await Task.Delay(1000);
        _resetPin.State = true;

        // Wait for the gpu to signal it's ready
        await WaitForHandshakeAsync();
    }

    public void EnqueueOutboundOperation(IFireAndForgetOperation operation)
    {
        _operations.Enqueue(operation);
    }

    public async ValueTask SendQueuedOutboundOperationsAsync()
    {
        var bufferBytesWritten = 0;
        while(_operations.TryDequeue(out var operationToSend))
        {
            var sizeRequired = _packetFramer.BufferSizeRequired(operationToSend);
            if (bufferBytesWritten + sizeRequired >= _buffer.Length)
            {
                // We have too much space, send what we have so far
                await WaitForHandshakeAsync();
                _spiBus.Write(_chipSelectPin, _buffer[..bufferBytesWritten]);
                bufferBytesWritten = 0;
            }

            bufferBytesWritten += _packetFramer.Encode(operationToSend, _buffer.AsSpan(bufferBytesWritten));
        }

        if (bufferBytesWritten > 0)
        {
            await WaitForHandshakeAsync();
            _spiBus.Write(_chipSelectPin, _buffer[..bufferBytesWritten]);
        }
    }

    public async ValueTask SendImmediateOperationAsync(IOperation operation)
    {
        var bytesWritten = _packetFramer.Encode(operation, _buffer.AsSpan());
        await WaitForHandshakeAsync();
        _spiBus.Write(_chipSelectPin, _buffer[..bytesWritten]);
    }

    public async ValueTask<TResponse?> ReadNextResponseAsync<TResponse>() where TResponse : class, IResponse, new()
    {
        await WaitForHandshakeAsync();
        
        // We only expect one single COBS packet, so we are guaranteed to be at most 255 bytes
        _spiBus.Read(_chipSelectPin, _buffer.AsSpan(0, 255));
        var result = _packetFramer.Decode(_buffer);

        if (result.InputBytesProcessed == 0)
        {
            // Incomplete packet received
            return null;
        }

        if (result.DecodedBytes.Length == 0)
        {
            // Undecodable packet
            return null;
        }
        
        var response = new TResponse();
        response.Deserialize(result.DecodedBytes.Span);

        return response;
    }

    private async Task WaitForHandshakeAsync()
    {
        var startedAt = DateTime.Now;
        while (!_handshakePin.State)
        {
            if (DateTime.Now - startedAt > TimeSpan.FromSeconds(5))
                throw new TimeoutException("Timeout waiting for SPI handshake");

            await Task.Yield();
        }
    }
}