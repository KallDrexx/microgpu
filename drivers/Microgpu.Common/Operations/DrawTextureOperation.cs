using System;

namespace Microgpu.Common.Operations;

public class DrawTextureOperation : IFireAndForgetOperation
{
    public required byte SourceTextureId { get; init; }
    public required byte TargetTextureId { get; init; }
    public required ushort SourceStartX { get; init; }
    public required ushort SourceStartY { get; init; }
    public required ushort SourceWidth { get; init; }
    public required ushort SourceHeight { get; init; }
    public required short TargetStartX { get; init; }
    public required short TargetStartY { get; init; }
    public bool IgnoreTransparency { get; init; }

    public int Serialize(Span<byte> bytes)
    {
        bytes[0] = 11;
        bytes[1] = SourceTextureId;
        bytes[2] = TargetTextureId;
        bytes[3] = (byte)(SourceStartX >> 8);
        bytes[4] = (byte)(SourceStartX & 0xFF);
        bytes[5] = (byte)(SourceStartY >> 8);
        bytes[6] = (byte)(SourceStartY & 0xFF);
        bytes[7] = (byte)(SourceWidth >> 8);
        bytes[8] = (byte)(SourceWidth & 0xFF);
        bytes[9] = (byte)(SourceHeight >> 8);
        bytes[10] = (byte)(SourceHeight & 0xFF);
        bytes[11] = (byte)(TargetStartX >> 8);
        bytes[12] = (byte)(TargetStartX & 0xFF);
        bytes[13] = (byte)(TargetStartY >> 8);
        bytes[14] = (byte)(TargetStartY & 0xFF);

        bytes[15] = 0;
        if (IgnoreTransparency)
        {
            bytes[15] |= 1;
        }

        return 16;
    }
}