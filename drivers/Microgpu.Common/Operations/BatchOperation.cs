using System;

namespace Microgpu.Common.Operations;

public class BatchOperation : IFireAndForgetOperation
{
    private readonly byte[] _buffer = new byte[1020]; 
    private int _offset = 0;
    
    public void AddOperation<T>(T operation) where T : IFireAndForgetOperation
    {
        var bytesAdded = operation.Serialize(_buffer.AsSpan(_offset + 2));
        _buffer[_offset] = (byte)(bytesAdded >> 8);
        _buffer[_offset + 1] = (byte)(bytesAdded & 0xFF);
        _offset += bytesAdded + 2;
    }

    public int Serialize(Span<byte> bytes)
    {
        bytes[0] = 7;
        bytes[1] = (byte)(_offset >> 8);
        bytes[2] = (byte)(_offset & 0xFF);
        _buffer.AsSpan(0, _offset).CopyTo(bytes[3..]);

        var totalBytes = _offset + 3;
        _offset = 0;

        return totalBytes;
    }
}