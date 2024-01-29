using System;

namespace Microgpu.Common;

/// <summary>
///     Used to constrain generics to a color type
/// </summary>
public interface IColorType
{
    /// <summary>
    ///     Writes the bytes for the color to the beginning of the provided byte span
    /// </summary>
    /// <returns>The number of bytes written to the span</returns>
    int WriteBytes(Span<byte> bytes);
   
    /// <summary>
    ///     Returns the size of the color in bytes
    /// </summary>
    /// <returns></returns>
    int GetSize();
}