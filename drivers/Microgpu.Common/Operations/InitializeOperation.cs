using System.Collections.Generic;

namespace Microgpu.Common.Operations
{
    public class InitializeOperation : IOperation
    {
        public byte FrameBufferScale { get; set; }
        
        public void Serialize(List<byte> bytes)
        {
            bytes.Clear();
            bytes.Add(1);
            bytes.Add(FrameBufferScale);
        }
    }
}