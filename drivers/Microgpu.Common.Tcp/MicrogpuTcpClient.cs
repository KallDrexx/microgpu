using System.Net.Sockets;
using Microgpu.Common.Operations;
using System;

namespace Microgpu.Common.Tcp;

public class MicrogpuTcpClient : IDisposable
{
    private readonly TcpClient _tcpClient = new();
    // private readonly Socket _socket = new(SocketType.Stream, ProtocolType.Tcp);
    private readonly string _host;
    private readonly int _port;
    private readonly byte[] _buffer = new byte[1026];
    private NetworkStream? _networkStream;
    
    public MicrogpuTcpClient(string host, int port)
    {
        _host = host;
        _port = port;
    }
    
    public async Task ConnectAsync()
    {
        await _tcpClient.ConnectAsync(_host, _port);
        _networkStream = _tcpClient.GetStream();
    }
    
    public async Task SendOperationAsync(IOperation operation)
    {
        if (!_tcpClient.Connected)
        {
            await ConnectAsync();
        }
        
        var byteCount = operation.Serialize(_buffer.AsSpan(2));
        
        var lengthByte1 = (byte)(byteCount >> 8);
        var lengthByte2 = (byte)(byteCount & 0xFF);

        _buffer[0] = lengthByte1;
        _buffer[1] = lengthByte2;
        
        await _networkStream!.WriteAsync(_buffer.AsMemory(0, byteCount + 2));
    }

    public void Dispose()
    {
        _networkStream?.Dispose();
        _tcpClient.Dispose();
    }
}