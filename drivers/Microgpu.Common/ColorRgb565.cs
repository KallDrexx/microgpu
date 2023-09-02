using System.Collections.Generic;

namespace Microgpu.Common
{
    public readonly struct ColorRgb565 : IColorType
    {
        public readonly ushort Value;
        
        public ColorRgb565(ushort value)
        {
            this.Value = value;
        }
        
        public static ColorRgb565 FromRgb565(byte red, byte green, byte blue)
        {
            return new ColorRgb565((ushort)((red << 11) | (green << 5) | blue));
        }

        public static ColorRgb565 FromRgb888(byte red, byte green, byte blue)
        {
            // convert them from 24bit space values to 16bit values
            red /= 8;
            green /= 4;
            blue /= 8;

            return FromRgb565(red, green, blue);
        }
        
        public void AppendBytes(List<byte> bytes)
        {
            bytes.Add((byte)(Value >> 8));
            bytes.Add((byte)(Value & 0xFF));
        }
    }
}