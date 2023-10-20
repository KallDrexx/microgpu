using Meadow.Foundation.Graphics;
using Meadow.Foundation.Graphics.Buffers;
using Microgpu.Common;
using Microgpu.Common.Operations;

namespace Microgpu.Sample.Common;

public class TextureManager
{
    private readonly Gpu _gpu;
    private readonly BufferRgb565[] _textures = new BufferRgb565[2];

    public TextureManager(Gpu gpu)
    {
        _gpu = gpu;
        var texture = LoadTexture();

        _textures[0] = SubTexture(texture, 69, 3, 23, 26); // tree
        _textures[1] = SubTexture(texture, 0, 48, 16, 16); // sun
    }

    public async Task SendTexturesToGpuAsync()
    {
        var operation = new SetTextureCountOperation { TextureCount = (byte)_textures.Length };
        await _gpu.SendFireAndForgetAsync(operation);

        for (var x = 0; x < _textures.Length; x++)
        {
            Console.WriteLine($"Sending texture {x} to the GPU");
            var textureId = (byte)x;
            var texture = _textures[x];
            await _gpu.SendFireAndForgetAsync(new DefineTextureOperation<ColorRgb565>
            {
                TextureId = textureId,
                Width = (ushort)texture.Width,
                Height = (ushort)texture.Height,
                TransparentColor = ColorRgb565.FromRgb888(255, 0, 255)
            });

            var bytesLeft = texture.Buffer.Length;
            while (bytesLeft > 0)
            {
                var bytesToSend = Math.Min(bytesLeft, 512);
                var startIndex = texture.Buffer.Length - bytesLeft;

                await _gpu.SendFireAndForgetAsync(new AppendTexturePixelsOperation
                {
                    TextureId = textureId,
                    PixelBytes = texture.Buffer.AsMemory(startIndex, bytesToSend)
                });

                bytesLeft -= bytesToSend;
            }
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
        imageBuffer.WriteBuffer(0, 0, image.DisplayBuffer);

        return imageBuffer;
    }
}