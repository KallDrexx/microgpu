using System;
using System.Collections.Generic;

namespace Microgpu.Common.Operations
{
    public class DrawTriangleOperation<TColor> : IFireAndForgetOperation where TColor : IColorType
    {
        public required ushort X0 { get; init; }
        public required ushort Y0 { get; init; }
        public required ushort X1 { get; init; }
        public required ushort Y1 { get; init; }
        public required ushort X2 { get; init; }
        public required ushort Y2 { get; init; }
        public required TColor Color { get; init; }
        
        public int Serialize(Span<byte> bytes)
        {
            bytes[0] = (3);
            bytes[1] = (byte)(X0 >> 8);
            bytes[2] = (byte)(X0 & 0xFF);
            bytes[3] = (byte)(Y0 >> 8);
            bytes[4] = (byte)(Y0 & 0xFF);
            bytes[5] = (byte)(X1 >> 8);
            bytes[6] = (byte)(X1 & 0xFF);
            bytes[7] = (byte)(Y1 >> 8);
            bytes[8] = (byte)(Y1 & 0xFF);
            bytes[9] = (byte)(X2 >> 8);
            bytes[10] = (byte)(X2 & 0xFF);
            bytes[11] = (byte)(Y2 >> 8);
            bytes[12] = (byte)(Y2 & 0xFF);
            
            return 13 + Color.WriteBytes(bytes[13..]);
        }
    }
}