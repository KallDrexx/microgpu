using System;

namespace Microgpu.Common.Operations;

public class BatchOperation : IFireAndForgetOperation
{
    private readonly byte[] _buffer = new byte[1020];
    private int _offset;

    public int Serialize(Span<byte> bytes)
    {
        if (bytes.Length < GetSize())
        {
            var message = $"Buffer had a size of {bytes.Length} but {GetSize()} was required.";
            throw new ArgumentException(message);
        }
        
        bytes[0] = 7;
        bytes[1] = (byte)(_offset >> 8);
        bytes[2] = (byte)(_offset & 0xFF);
        _buffer.AsSpan(0, _offset).CopyTo(bytes[3..]);

        var totalBytes = _offset + 3;
        _offset = 0;

        return totalBytes;
    }

    public int GetSize()
    {
        return _offset + 3;
    }

    /// <summary>
    /// Adds the operation into the batch. 
    /// </summary>
    /// <returns>True if the operation was added, false if there was not enough space</returns>
    public bool AddOperation<T>(T operation) where T : IFireAndForgetOperation
    {
        var requiredSize = operation.GetSize();
        if (_offset + requiredSize + 2 > _buffer.Length)
        {
            return false;
        }
        
        var bytesAdded = operation.Serialize(_buffer.AsSpan(_offset + 2));
        _buffer[_offset] = (byte)(bytesAdded >> 8);
        _buffer[_offset + 1] = (byte)(bytesAdded & 0xFF);
        _offset += bytesAdded + 2;

        return true;
    }
    
    public bool HasAnyOperations()
    {
        return _offset > 0;
    }
}