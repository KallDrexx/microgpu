using System;
using Microgpu.Common.Operations;

namespace Microgpu.Common;

public readonly record struct PacketDecodeResult(ReadOnlyMemory<byte> DecodedBytes, int InputBytesProcessed);
   
/// <summary>
/// Uses Consistent Overhead Byte Stuffing to serialize an operation into bytes, or to read a packet out of
/// a buffer.
/// </summary>
public class PacketFramer
{
    private readonly byte[] _decodeBuffer = new byte[255];
    private const int MaxMessageSize = 250;

    /// <summary>
    /// Gets the number of bytes required to encode this operation into the buffer
    /// </summary>
    public int BufferSizeRequired(IOperation operation)
    {
        return operation.GetSize() + 4;
    }
    
    /// <summary>
    /// Serialize and encodes the operation into the provided buffer
    /// </summary>
    /// <returns>The total number of bytes written to the output buffer</returns>
    public int Encode(IOperation operation, Span<byte> outputBuffer)
    {
        var size = operation.GetSize();
        if (size > MaxMessageSize)
        {
            var message = $"Operation {operation.GetType().Name} serializes into {size} bytes, which is more " +
                          $"than the allowed {MaxMessageSize}.";
            throw new InvalidOperationException(message);
        }

        // Make sure we have enough bytes for the message + initial offset + 2 checksum + trailing zero bytes
        if (outputBuffer.Length < size + 4)
        {
            var message = $"Operation {operation.GetType().Name} serializes into {size} bytes, which is larger " +
                          $"than the provided buffer of {outputBuffer.Length} bytes";
            throw new InvalidOperationException(message);
        }

        var bytesWritten = operation.Serialize(outputBuffer[1..]);
        var checksum = CalculateChecksum(outputBuffer[1..(bytesWritten + 1)]);
        outputBuffer[bytesWritten + 1] = (byte)(checksum >> 8);
        outputBuffer[bytesWritten + 2] = (byte)(checksum & 0x00FF);
        outputBuffer[bytesWritten + 3] = 0;
        
        // Calculate offsets to each zero as we iterate through the bytes
        var lastZeroIndex = 0;
        for (var x = 1; x < bytesWritten + 4; x++)
        {
            if (outputBuffer[x] == 0)
            {
                outputBuffer[lastZeroIndex] = (byte)(x - lastZeroIndex);
                lastZeroIndex = x;
            }
        }

        return bytesWritten + 4;
    }

    public PacketDecodeResult Decode(ReadOnlySpan<byte> buffer)
    {
        var zeroIndex = (int?)null;
        for (var x = 0; x < buffer.Length; x++)
        {
            if (buffer[x] == 0)
            {
                zeroIndex = x;
                break;
            }
        }

        if (zeroIndex == null)
        {
            // No zeros were found, not a complete packet (or corrupted).  
            return new PacketDecodeResult(null, 0);
        }

        if (zeroIndex.Value <= 3)
        {
            // Complete packet but not enough bytes to be valid
            return new PacketDecodeResult(null, zeroIndex.Value + 1);
        }
        
        if (zeroIndex.Value >= _decodeBuffer.Length)
        {
            // Too many bytes in the packet, possibly zeros missing due to corruption
            return new PacketDecodeResult(null, zeroIndex.Value + 1);
        }

        var amountToCopy = zeroIndex.Value + 1;
        buffer[..amountToCopy].CopyTo(_decodeBuffer);
        
        // Use offsets to correct zeros
        var index = 0;
        while (true)
        {
            var nextZeroIndex = index + _decodeBuffer[index];
            if (nextZeroIndex >= amountToCopy)
            {
                // Went past the buffer. The offsets are wrong so the message is corrupted
                return new PacketDecodeResult(null, amountToCopy);
            }

            _decodeBuffer[index] = 0;
            if (nextZeroIndex == amountToCopy - 1)
            {
                // Hit the final zero
                break;
            }
            
            index = nextZeroIndex;
        }

        var expectedChecksum = (ushort)((_decodeBuffer[amountToCopy - 3] << 8) | _decodeBuffer[amountToCopy - 2]);
        var calculatedChecksum = CalculateChecksum(_decodeBuffer[1..(amountToCopy - 3)]);
        if (expectedChecksum != calculatedChecksum)
        {
            // Checksum mismatch, so most likely a corrupted packet
            return new PacketDecodeResult(null, amountToCopy);
        }

        var memoryBuffer = _decodeBuffer.AsMemory(1..(amountToCopy - 3));
        return new PacketDecodeResult(memoryBuffer, amountToCopy);
    }

    private ushort CalculateChecksum(ReadOnlySpan<byte> bytes)
    {
        ushort checksum = 0;
        for (var x = 0; x < bytes.Length; x++)
        {
            unchecked
            {
                checksum += bytes[x];
            }
        }

        return checksum;
    }
}