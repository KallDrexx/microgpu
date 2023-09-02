using System.Collections.Generic;

namespace Microgpu.Common.Operations
{
    public class DrawTriangleOperation<TColor> : IOperation where TColor : IColorType
    {
        public required ushort X0 { get; init; }
        public required ushort Y0 { get; init; }
        public required ushort X1 { get; init; }
        public required ushort Y1 { get; init; }
        public required ushort X2 { get; init; }
        public required ushort Y2 { get; init; }
        public required TColor Color { get; init; }
        
        public void Serialize(List<byte> bytes)
        {
            bytes.Clear();
            bytes.Add(3);
            bytes.Add((byte)(X0 >> 8));
            bytes.Add((byte)(X0 & 0xFF));
            bytes.Add((byte)(Y0 >> 8));
            bytes.Add((byte)(Y0 & 0xFF));
            bytes.Add((byte)(X1 >> 8));
            bytes.Add((byte)(X1 & 0xFF));
            bytes.Add((byte)(Y1 >> 8));
            bytes.Add((byte)(Y1 & 0xFF));
            bytes.Add((byte)(X2 >> 8));
            bytes.Add((byte)(X2 & 0xFF));
            bytes.Add((byte)(Y2 >> 8));
            bytes.Add((byte)(Y2 & 0xFF));
            
            Color.AppendBytes(bytes);
        }
    }
}