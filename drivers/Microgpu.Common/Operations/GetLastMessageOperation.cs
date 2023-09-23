using System;
using Microgpu.Common.Responses;

namespace Microgpu.Common.Operations
{
    public class GetLastMessageOperation : IResponsiveOperation<LastMessageResponse>
    {
        public int Serialize(Span<byte> bytes)
        {
            bytes[0] = 5;

            return 1;
        }
    }
}