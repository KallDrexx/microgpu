using System;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Threading.Tasks;
using Microgpu.Common.Operations;
using Microgpu.Common.Responses;

namespace Microgpu.Common.Comms;

public class TcpGpuCommunication : IGpuCommunication, IDisposable
{
    private readonly byte[] _buffer = new byte[1026];
    private readonly string _host;
    private readonly int _port;
    private readonly TcpClient _tcpClient = new();
    private readonly Queue<IFireAndForgetOperation> _operations = new();
    private readonly PacketFramer _packetFramer = new();
    private NetworkStream? _networkStream;

    public TcpGpuCommunication(string host, int port)
    {
        _host = host;
        _port = port;
    }

    public void Dispose()
    {
        _networkStream?.Dispose();
        _tcpClient.Dispose();
    }

    public async ValueTask ResetAsync()
    {
        await ConnectAsync();

        // TODO: SDL server reset rebuilds tcp connection, which won't work
        // byte[] buffer = { 189, 0x09, 0x13, 0xac };
        // await SendDataAsync(buffer.AsMemory());
        //
        // await Task.Delay(100);
    }

    public void EnqueueOutboundOperation(IFireAndForgetOperation operation)
    {
        _operations.Enqueue(operation);
    }

    public async ValueTask SendQueuedOutboundOperationsAsync()
    {
        while (_operations.TryDequeue(out var operationToSend))
        {
            await SendDataAsync(operationToSend);
        }
    }

    public async ValueTask SendImmediateOperationAsync(IOperation operation)
    {
        await SendDataAsync(operation);
    }

    public async ValueTask<TResponse?> ReadNextResponseAsync<TResponse>() where TResponse : class, IResponse, new()
    {
        // We should only have one response at a time, since each response is a 
        // direct reaction to a non-fire and forget question. So just keep reading
        // until we get a full packet or read more than 255 bytes.
        var totalRead = 0;
        while (true)
        {
            var bytesRead = await _networkStream!.ReadAsync(_buffer.AsMemory(totalRead));
            totalRead += bytesRead;
            
            // Did we get a complete packet?
            var result = _packetFramer.Decode(_buffer[..totalRead]);
            if (result.InputBytesProcessed == 0)
            {
                // We didn't have a complete packet so keep reading
                continue;
            }
            
            // We found a packet boundary
            if (result.DecodedBytes.Length == 0)
            {
                // Corrupted packet
                return null;
            }

            var response = new TResponse();
            response.Deserialize(result.DecodedBytes.Span);
            return response;
        }
    }

    private async Task SendDataAsync(IOperation operation)
    {
        var bytesWritten = _packetFramer.Encode(operation, _buffer.AsSpan());
        if (bytesWritten > 0)
        {
            await _networkStream!.WriteAsync(_buffer.AsMemory(0, bytesWritten));
        }
    }

    private async ValueTask ConnectAsync()
    {
        if (_tcpClient.Connected) return;

        await _tcpClient.ConnectAsync(_host, _port);
        _networkStream = _tcpClient.GetStream();
    }
}