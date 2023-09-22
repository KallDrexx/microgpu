using System;
using System.Collections.Generic;

namespace Microgpu.Common.Operations
{
    public class GetLastMessageOperation : IResponsiveOperation
    {
        public int Serialize(Span<byte> bytes)
        {
            bytes[0] = 5;

            return 1;
        }
    }
}