using System;

namespace Microgpu.Common.Operations;

public class DrawRectangleOperation<TColor> : IFireAndForgetOperation where TColor : IColorType
{
    public required ushort StartX { get; init; }
    public required ushort StartY { get; init; }
    public required ushort Width { get; init; }
    public required ushort Height { get; init; }
    public required TColor Color { get; init; }

    public int Serialize(Span<byte> bytes)
    {
        bytes[0] = 2;
        bytes[1] = (byte)(StartX >> 8);
        bytes[2] = (byte)(StartX & 0xFF);
        bytes[3] = (byte)(StartY >> 8);
        bytes[4] = (byte)(StartY & 0xFF);
        bytes[5] = (byte)(Width >> 8);
        bytes[6] = (byte)(Width & 0xFF);
        bytes[7] = (byte)(Height >> 8);
        bytes[8] = (byte)(Height & 0xFF);

        return 9 + Color.WriteBytes(bytes[9..]);
    }
}