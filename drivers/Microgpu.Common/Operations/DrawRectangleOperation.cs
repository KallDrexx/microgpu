using System;

namespace Microgpu.Common.Operations;

public class DrawRectangleOperation<TColor> : IFireAndForgetOperation where TColor : IColorType
{
    public required byte TextureId { get; init; }
    public required ushort StartX { get; init; }
    public required ushort StartY { get; init; }
    public required ushort Width { get; init; }
    public required ushort Height { get; init; }
    public required TColor Color { get; init; }

    public int Serialize(Span<byte> bytes)
    {
        bytes[0] = 2;
        bytes[1] = TextureId;
        bytes[2] = (byte)(StartX >> 8);
        bytes[3] = (byte)(StartX & 0xFF);
        bytes[4] = (byte)(StartY >> 8);
        bytes[5] = (byte)(StartY & 0xFF);
        bytes[6] = (byte)(Width >> 8);
        bytes[7] = (byte)(Width & 0xFF);
        bytes[8] = (byte)(Height >> 8);
        bytes[9] = (byte)(Height & 0xFF);

        return 10 + Color.WriteBytes(bytes[10..]);
    }

    public int GetSize()
    {
        return 10 + Color.GetSize();
    }
}