using System;

namespace Microgpu.Common.Operations;

public class DrawTextureOperation : IFireAndForgetOperation
{
    public required byte TextureId { get; init; }
    public required short X { get; init; }
    public required short Y { get; init; }

    public int Serialize(Span<byte> bytes)
    {
        bytes[0] = 11;
        bytes[1] = TextureId;
        bytes[2] = (byte)(X >> 8);
        bytes[3] = (byte)(X & 0xFF);
        bytes[4] = (byte)(Y >> 8);
        bytes[5] = (byte)(Y & 0xFF);

        return 6;
    }
}