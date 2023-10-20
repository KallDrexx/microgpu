using System;

namespace Microgpu.Common.Operations;

public class InitializeOperation : IFireAndForgetOperation
{
    public byte FrameBufferScale { get; set; }

    public int Serialize(Span<byte> bytes)
    {
        bytes[0] = 1;
        bytes[1] = FrameBufferScale;

        return 2;
    }
}