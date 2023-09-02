using System.Collections.Generic;

namespace Microgpu.Common.Operations
{
    public class DrawRectangleOperation<TColor> : IOperation where TColor : IColorType
    {
        public required ushort StartX { get; init; }
        public required ushort StartY { get; init; }
        public required ushort Width { get; init; }
        public required ushort Height { get; init; }
        public required TColor Color { get; init; }
        
        public void Serialize(List<byte> bytes)
        {
            bytes.Clear();
            bytes.Add(2);
            bytes.Add((byte)(StartX >> 8));
            bytes.Add((byte)(StartX & 0xFF));
            bytes.Add((byte)(StartY >> 8));
            bytes.Add((byte)(StartY & 0xFF));
            bytes.Add((byte)(Width >> 8));
            bytes.Add((byte)(Width & 0xFF));
            bytes.Add((byte)(Height >> 8));
            bytes.Add((byte)(Height & 0xFF));
            
            Color.AppendBytes(bytes);
        }
    }
}