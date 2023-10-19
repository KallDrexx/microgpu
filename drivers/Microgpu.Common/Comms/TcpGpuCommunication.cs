using System;
using System.Net.Sockets;
using System.Threading.Tasks;

namespace Microgpu.Common.Comms;

public class TcpGpuCommunication : IGpuCommunication, IDisposable
{
    private readonly byte[] _buffer = new byte[1026];
    private readonly string _host;
    private readonly int _port;
    private readonly TcpClient _tcpClient = new();
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

    public async Task ResetAsync()
    {
        await ConnectAsync();

        // TODO: SDL server reset rebuilds tcp connection, which won't work
        // byte[] buffer = { 189, 0x09, 0x13, 0xac };
        // await SendDataAsync(buffer.AsMemory());
        //
        // await Task.Delay(100);
    }

    public async Task SendDataAsync(Memory<byte> data)
    {
        _buffer[0] = (byte)(data.Length >> 8);
        _buffer[1] = (byte)(data.Length & 0xFF);
        data.CopyTo(_buffer.AsMemory(2));

        await _networkStream!.WriteAsync(_buffer.AsMemory(0, data.Length + 2));
    }

    public async Task<int> ReadDataAsync(Memory<byte> data)
    {
        var bytesRead = await _networkStream!.ReadAsync(_buffer, 0, 2);
        if (bytesRead != 2) throw new InvalidOperationException("Expected 2 bytes for length");

        var length = (ushort)((_buffer[0] << 8) | _buffer[1]);
        var totalBytesRead = 0;
        while (totalBytesRead < length)
        {
            var bytesRemaining = length - totalBytesRead;
            bytesRead = await _networkStream.ReadAsync(_buffer, 0, bytesRemaining);
            _buffer.AsMemory(0, bytesRead).CopyTo(data[totalBytesRead..]);
            totalBytesRead += bytesRead;
        }

        return totalBytesRead;
    }

    private async Task ConnectAsync()
    {
        if (_tcpClient.Connected) return;

        await _tcpClient.ConnectAsync(_host, _port);
        _networkStream = _tcpClient.GetStream();
    }
}