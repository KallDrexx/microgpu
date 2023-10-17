using System;

namespace Microgpu.Common.Operations;

public class AppendTexturePixelsOperation : IFireAndForgetOperation
{
    public required byte TextureId { get; init; }
    public required Memory<byte> Pixels { get; init; }
    
    public int Serialize(Span<byte> bytes)
    {
        if (bytes.Length < 3 + Pixels.Length)
        {
            throw new ArgumentException("Not enough space to serialize operation");
        }
        
        bytes[0] = 10;
        bytes[1] = (byte)(Pixels.Length << 8);
        bytes[2] = (byte)Pixels.Length;

        for (var x = 0; x < Pixels.Length; x++)
        {
            bytes[3 + x] = Pixels.Span[x];
        }

        return 3 + Pixels.Length;
    }
}