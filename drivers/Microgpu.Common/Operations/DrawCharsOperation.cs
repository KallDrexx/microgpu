using System;

namespace Microgpu.Common.Operations;

public class DrawCharsOperation<TColor> : IFireAndForgetOperation where TColor : IColorType
{
    public required string Text { get; init; }
    public required TColor Color { get; init; }
    public required Font Font { get; init; }
    public required byte TextureId { get; init; }
    public required ushort StartX { get; init; }
    public required ushort StartY { get; init; }
        
    public int Serialize(Span<byte> bytes)
    {
        if (Text?.Length > 255)
        {
            var message = $"Attempting to draw {Text.Length} chars which exceeds the max of 255";
            throw new InvalidOperationException(message);
        }
        
        var size = GetSize();
        if (bytes.Length < size)
        {
            var message = $"DrawChars requires {size} bytes, but the buffer only has {bytes.Length}";
            throw new InvalidOperationException(message);
        }

        bytes[0] = 12;
        bytes[1] = (byte)Font;
        bytes[2] = TextureId;

        var writtenBytes = Color.WriteBytes(bytes[3..]);
        var byteIndex = 3 + writtenBytes;
        bytes[byteIndex] = (byte)(StartX >> 8);
        bytes[++byteIndex] = (byte)(StartX & 0xFF);
        bytes[++byteIndex] = (byte)(StartY >> 8);
        bytes[++byteIndex] = (byte)(StartY & 0xFF);
        bytes[++byteIndex] = (byte)(Text?.Length ?? 0);

        foreach (var character in Text?.ToCharArray() ?? Array.Empty<char>())
        {
            bytes[++byteIndex] = (byte)character;
        }

        return byteIndex + 1;
    }

    public int GetSize()
    {
        return 8 + Color.GetSize() + (Text?.Length ?? 0);
    }
}