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
        while (CollectQueuedOperations() is {} operationToSend)
        {
            var bytesWritten = operationToSend.Serialize(_buffer[2..]);
            await SendDataAsync(bytesWritten);
        }
    }

    public async ValueTask SendImmediateOperationAsync(IOperation operation)
    {
        var bytesWritten = operation.Serialize(_buffer[2..]);
        await SendDataAsync(bytesWritten);
    }

    public async ValueTask<TResponse?> ReadNextResponseAsync<TResponse>() where TResponse : class, IResponse, new()
    {
        var bytesRead = await _networkStream!.ReadAsync(_buffer, 0, 2);
        if (bytesRead != 2) throw new InvalidOperationException("Expected 2 bytes for length");

        var length = (ushort)((_buffer[0] << 8) | _buffer[1]);
        var totalBytesRead = 0;
        while (totalBytesRead < length)
        {
            var bytesRemaining = length - totalBytesRead;
            bytesRead = await _networkStream.ReadAsync(_buffer, totalBytesRead, bytesRemaining);
            totalBytesRead += bytesRead;
        }

        var response = new TResponse();
        response.Deserialize(_buffer[..totalBytesRead]);

        return response;
    }

    private async Task SendDataAsync(int length)
    {
        _buffer[0] = (byte)(length >> 8);
        _buffer[1] = (byte)(length & 0xFF);

        await _networkStream!.WriteAsync(_buffer.AsMemory(0, length + 2));
    }

    private async ValueTask ConnectAsync()
    {
        if (_tcpClient.Connected) return;

        await _tcpClient.ConnectAsync(_host, _port);
        _networkStream = _tcpClient.GetStream();
    }

    private IOperation? CollectQueuedOperations()
    {
        if (_operations.Count == 0)
        {
            return null;
        }

        if (_operations.Count == 1)
        {
            return _operations.Dequeue();
        }

        var batch = new BatchOperation();
        while (_operations.Count > 0)
        {
            var operation = _operations.Peek();
            if (!batch.AddOperation(operation))
            {
                // Not enough space in the batch
                break;
            }

            _operations.Dequeue();
        }

        return batch;
    }
}