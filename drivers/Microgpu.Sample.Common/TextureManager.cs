using Meadow.Foundation.Graphics;
using Meadow.Foundation.Graphics.Buffers;
using Microgpu.Common;
using Microgpu.Common.Operations;

namespace Microgpu.Sample.Common;

public class TextureManager
{
    public record Texture(byte Id, ushort Width, ushort Height, BufferRgb565 Buffer);
    private readonly Gpu _gpu;
    
    public Texture[] Textures { get; } = new Texture[2];

    public TextureManager(Gpu gpu)
    {
        _gpu = gpu;
        var texture = LoadTexture();
        
        Textures[0] = new Texture(1, 23, 26, SubTexture(texture, 69, 3, 23, 26)); // tree
        Textures[1] = new Texture(2, 16, 16, SubTexture(texture, 0, 48, 16, 16)); // sun
    }

    public async Task SendTexturesToGpuAsync()
    {
        foreach (var texture in Textures)
        {
            Console.WriteLine($"Sending texture {texture.Id} to the GPU");
            _gpu.EnqueueFireAndForgetAsync(new DefineTextureOperation<ColorRgb565>
            {
                TextureId = texture.Id,
                Width = texture.Width,
                Height = texture.Height,
                TransparentColor = ColorRgb565.FromRgb888(255, 0, 255)
            });
            
            var bytesLeft = texture.Buffer.Buffer.Length;
            while (bytesLeft > 0)
            {
                var bytesToSend = Math.Min(bytesLeft, 512);
                var startIndex = texture.Buffer.Buffer.Length - bytesLeft;

                _gpu.EnqueueFireAndForgetAsync(new AppendTexturePixelsOperation
                {
                    TextureId = texture.Id,
                    PixelBytes = texture.Buffer.Buffer.AsMemory(startIndex, bytesToSend)
                });

                bytesLeft -= bytesToSend;
            }

            await _gpu.SendQueuedOperationsAsync();
        }

        Console.WriteLine("All textures sent to GPU");
    }

    private static BufferRgb565 SubTexture(BufferRgb565 spriteSheet, int x, int y, int width, int height)
    {
        var imageBuffer = new BufferRgb565(width, height);
        for (var yIndex = 0; yIndex < height; yIndex++)
        for (var xIndex = 0; xIndex < width; xIndex++)
        {
            var spriteSheetX = x + xIndex;
            var spriteSheetY = y + yIndex;
            var spriteSheetIndex = spriteSheetY * spriteSheet.Width * 2 + spriteSheetX * 2;
            var imageIndex = yIndex * width * 2 + xIndex * 2;
            imageBuffer.Buffer[imageIndex] = spriteSheet.Buffer[spriteSheetIndex];
            imageBuffer.Buffer[imageIndex + 1] = spriteSheet.Buffer[spriteSheetIndex + 1];
        }

        return imageBuffer;
    }

    private static BufferRgb565 LoadTexture()
    {
        var image = Image.LoadFromFile(Path.Combine(AppContext.BaseDirectory, "spritesheet.bmp"));
        var imageBuffer = new BufferRgb565(image.Width, image.Height);
        imageBuffer.WriteBuffer(0, 0, image.DisplayBuffer!);

        return imageBuffer;
    }
}