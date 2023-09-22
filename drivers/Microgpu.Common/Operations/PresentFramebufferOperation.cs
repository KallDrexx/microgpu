using System;

namespace Microgpu.Common.Operations;

public class PresentFramebufferOperation : IFireAndForgetOperation
{
    public int Serialize(Span<byte> bytes)
    {
        bytes[0] = 6;

        return 1;
    }
}