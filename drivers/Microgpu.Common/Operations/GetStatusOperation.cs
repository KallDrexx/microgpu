using System;

namespace Microgpu.Common.Operations
{
    public class GetStatusOperation : IResponsiveOperation
    {
        public int Serialize(Span<byte> bytes)
        {
            bytes[0] = 4;
            
            return 1;
        }
    }
}