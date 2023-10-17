using System.Numerics;
using Meadow.Foundation.Graphics;
using Meadow.Foundation.Graphics.Buffers;
using Microgpu.Common;

namespace Microgpu.Sample.Common;

public class BouncingTexture
{
    private enum HorizontalDirection { Left, Right }
    private enum VerticalDirection { Up, Down }

    private const int Speed = 50;
    
    private readonly BufferRgb565 _texture;
    private Vector2 _position;
    private HorizontalDirection _horizontalDirection = HorizontalDirection.Right;
    private VerticalDirection _verticalDirection = VerticalDirection.Down;

    private BouncingTexture(Gpu gpu)
    {
        _texture = LoadTexture();

        var random = new Random();
        _position = new Vector2(
            (float)random.NextDouble() * gpu.FrameBufferResolution!.Value.X,
            (float)random.NextDouble() * gpu.FrameBufferResolution!.Value.Y);
    }
    
    public static async Task<BouncingTexture> CreateAsync(Gpu gpu)
    {
        gpu = gpu ?? throw new ArgumentNullException(nameof(gpu));

        if (!gpu.IsInitialized)
        {
            throw new InvalidOperationException("GPU must be initialized before creating a bouncing texture");
        }

        throw new NotImplementedException();
    }
    
    private static BufferRgb565 LoadTexture()
    {
        var image = Image.LoadFromFile(Path.Combine(AppContext.BaseDirectory, "spritesheet.bmp"));
        var imageBuffer = new BufferRgb565(23, 26);
        imageBuffer.WriteBuffer(69, 3, image.DisplayBuffer);

        return imageBuffer;
    }
}