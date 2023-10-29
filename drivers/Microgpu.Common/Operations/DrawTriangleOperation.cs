using System;

namespace Microgpu.Common.Operations;

public class DrawTriangleOperation<TColor> : IFireAndForgetOperation where TColor : IColorType
{
    public required byte TextureId { get; init; }
    public required ushort X0 { get; init; }
    public required ushort Y0 { get; init; }
    public required ushort X1 { get; init; }
    public required ushort Y1 { get; init; }
    public required ushort X2 { get; init; }
    public required ushort Y2 { get; init; }
    public required TColor Color { get; init; }

    public int Serialize(Span<byte> bytes)
    {
        bytes[0] = 3;
        bytes[1] = TextureId;
        bytes[2] = (byte)(X0 >> 8);
        bytes[3] = (byte)(X0 & 0xFF);
        bytes[4] = (byte)(Y0 >> 8);
        bytes[5] = (byte)(Y0 & 0xFF);
        bytes[6] = (byte)(X1 >> 8);
        bytes[7] = (byte)(X1 & 0xFF);
        bytes[8] = (byte)(Y1 >> 8);
        bytes[9] = (byte)(Y1 & 0xFF);
        bytes[10] = (byte)(X2 >> 8);
        bytes[11] = (byte)(X2 & 0xFF);
        bytes[12] = (byte)(Y2 >> 8);
        bytes[13] = (byte)(Y2 & 0xFF);

        return 14 + Color.WriteBytes(bytes[14..]);
    }
}