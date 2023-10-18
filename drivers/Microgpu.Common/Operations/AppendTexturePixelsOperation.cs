using System;

namespace Microgpu.Common.Operations;

public class AppendTexturePixelsOperation : IFireAndForgetOperation
{
    public required byte TextureId { get; init; }
    public required Memory<byte> PixelBytes { get; init; }
    
    public int Serialize(Span<byte> bytes)
    {
        var bpp = 2; // TODO: this should be based on color mode
        var pixelLength = PixelBytes.Length / 2;
        
        if (bytes.Length < 3 + PixelBytes.Length)
        {
            throw new ArgumentException("Not enough space to serialize operation");
        }
        
        bytes[0] = 10;
        bytes[1] = TextureId;
        bytes[2] = (byte)(pixelLength >> 8);
        bytes[3] = (byte)pixelLength;

        for (var x = 0; x < PixelBytes.Length; x++)
        {
            bytes[4 + x] = PixelBytes.Span[x];
        }

        return 4 + PixelBytes.Length;
    }
}