using Meadow;
using Microgpu.Common;

namespace Glade2d.Microgpu;

internal static class ColorExtensions
{
    public static ColorRgb565 ToColorRgb565(this Color color)
    {
        return ColorRgb565.FromRgb888(color.R, color.G, color.B);
    }
}