using System.Collections.Generic;

namespace Microgpu.Common.Operations
{
    public class GetStatusOperation : IOperation
    {
        public void Serialize(List<byte> bytes)
        {
            bytes.Clear();
            bytes.Add(4);
        }
    }
}