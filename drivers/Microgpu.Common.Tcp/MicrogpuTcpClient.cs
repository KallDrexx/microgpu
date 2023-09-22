using System.Net.Sockets;
using Microgpu.Common.Operations;

namespace Microgpu.Common.Tcp;

public class MicrogpuTcpClient : IDisposable
{
    private readonly Socket _socket = new(SocketType.Stream, ProtocolType.Tcp);
    private readonly string _host;
    private readonly int _port;
    private readonly byte[] _buffer = new byte[1026]; 
    
    public MicrogpuTcpClient(string host, int port)
    {
        _host = host;
        _port = port;
        
        _socket.SendTimeout = 1000;
        _socket.ReceiveTimeout = 1000;
    }
    
    public async Task ConnectAsync()
    {
        await _socket.ConnectAsync(_host, _port);
    }
    
    public async Task SendOperationAsync(IOperation operation)
    {
        if (!_socket.Connected)
        {
            await ConnectAsync();
        }
        
        var byteCount = operation.Serialize(_buffer[2..]);
        
        var lengthByte1 = (byte)(byteCount >> 8);
        var lengthByte2 = (byte)(byteCount & 0xFF);

        _buffer[0] = lengthByte1;
        _buffer[1] = lengthByte2;
        
        await _socket.SendAsync(_buffer.ToArray(), SocketFlags.None);
    }

    public void Dispose()
    {
        _socket.Dispose();
    }
}