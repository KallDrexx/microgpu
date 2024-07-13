using System.Text;
using Microgpu.Common;
using Microgpu.Common.Operations;

namespace Microgpu.Sample.Common.Samples;

public class FramerateDisplay
{
    private const Font DisplayFont = Font.Font12X16;
    private readonly StringBuilder _displayString = new();
    private readonly int[] _fpsValues = new int[10];
    private readonly Gpu _gpu;
    private int _nextFpsIndex = 0;

    public FramerateDisplay(Gpu gpu)
    {
        _gpu = gpu;
    }

    public void RunNextFrame(TimeSpan timeSinceLastFrame)
    {
        if (timeSinceLastFrame == TimeSpan.Zero)
        {
            return; // First frame, so ignore
        }
        
        var fps = (int)(1 / timeSinceLastFrame.TotalSeconds);
        _fpsValues[_nextFpsIndex] = fps;
        if (++_nextFpsIndex >= _fpsValues.Length)
        {
            _nextFpsIndex = 0;
        }

        var average = _fpsValues.Average();
        
        _displayString.Clear();
        _displayString.AppendFormat("{0:F0} fps", average);

        _gpu.EnqueueFireAndForgetAsync(new DrawCharsOperation<ColorRgb565>
        {
            Color = ColorRgb565.FromRgb888(255, 255, 255),
            Font = DisplayFont,
            StartX = 5,
            StartY = 5,
            TextureId = 0,
            Text = _displayString.ToString(),
        });
    }
}