using System;
using System.Text;

namespace Microgpu.Common;

public enum Font
{
    Font8X12 = 5,
    Font12X16 = 7,
}

public static class FontExtensions
{
    public static int CharWidth(this Font font)
    {
        return font switch
        {
            Font.Font8X12 => 8,
            Font.Font12X16 => 12,
            _ => throw new NotSupportedException($"Unknown font {font}")
        };
    }

    public static int CharHeight(this Font font)
    {
        return font switch
        {
            Font.Font8X12 => 12,
            Font.Font12X16 => 16,
            _ => throw new NotSupportedException($"Unknown font {font}")
        };
    }

    public static (int, int) GetSizeOnMeadow(this string str, Font font)
    {
        var width = str.Length * font.CharWidth();
        var height = font.CharHeight();
        return (width, height);
    }

    public static (int, int) GetSizeOnMeadow(this StringBuilder stringBuilder, Font font)
    {
        var width = stringBuilder.Length * font.CharWidth();
        var height = font.CharWidth();

        return (width, height);
    }
}