using System;
using Microgpu.Common.Responses;

namespace Microgpu.Common.Operations;

public class GetStatusOperation : IResponsiveOperation<StatusResponse>
{
    public int Serialize(Span<byte> bytes)
    {
        bytes[0] = 4;

        return 1;
    }

    public int GetSize()
    {
        return 1;
    }
}