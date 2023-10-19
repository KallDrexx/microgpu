using System;

namespace Microgpu.Common.Operations;

public class SetTextureCountOperation : IFireAndForgetOperation
{
    public byte TextureCount { get; set; }

    public int Serialize(Span<byte> bytes)
    {
        bytes[0] = 8;
        bytes[1] = TextureCount;

        return 2;
    }
}