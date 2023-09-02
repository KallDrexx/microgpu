using System.Collections.Generic;

namespace Microgpu.Common.Operations;

public class ResetOperation : IOperation
{
    public void Serialize(List<byte> bytes)
    {
        bytes.Clear();
        bytes.Add(189);
        
        // Magic numbers
        bytes.Add(0x09);
        bytes.Add(0x13);
        bytes.Add(0xac);
    }
}