using System;

namespace Microgpu.Common.Operations;

public class DefineTextureOperation<TColor> : IFireAndForgetOperation
    where TColor : IColorType
{
    public required byte TextureId { get; init; }
    public required ushort Width { get; init; }
    public required ushort Height { get; init; }
    public required TColor TransparentColor { get; init; }

    public int Serialize(Span<byte> bytes)
    {
        bytes[0] = 9;
        bytes[1] = TextureId;
        bytes[2] = (byte)(Width >> 8);
        bytes[3] = (byte)(Width & 0xFF);
        bytes[4] = (byte)(Height >> 8);
        bytes[5] = (byte)(Height & 0xFF);

        return 6 + TransparentColor.WriteBytes(bytes[6..]);
    }

    public int GetSize()
    {
        return 6 + TransparentColor.GetSize();
    }
}