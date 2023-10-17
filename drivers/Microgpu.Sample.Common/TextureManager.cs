using Meadow.Foundation.Graphics;
using Meadow.Foundation.Graphics.Buffers;
using Microgpu.Common;
using Microgpu.Common.Operations;

namespace Microgpu.Sample.Common;

public class TextureManager
{
    private readonly Gpu _gpu;
    private readonly BufferRgb565[] _textures = new BufferRgb565[1];

    public int TreeTextureId { get; } = 0;

    public TextureManager(Gpu gpu)
    {
        _gpu = gpu;
        var texture = LoadTexture();
        
        _textures[0] = SubTexture(texture, 69, 3, 23, 26); // tree
    }

    public async Task SendTexturesToGpuAsync()
    {
        var operation = new SetTextureCountOperation { TextureCount = (byte)_textures.Length };
        await _gpu.SendFireAndForgetAsync(operation);

        for (var x = 0; x < _textures.Length; x++)
        {
            var textureId = (byte)x;
            var texture = _textures[x];
            await _gpu.SendFireAndForgetAsync(new DefineTextureOperation<ColorRgb565>
            {
                TextureId = textureId,
                Width = (ushort)texture.Width,
                Height = (ushort)texture.Height,
                TransparentColor = ColorRgb565.FromRgb888(255, 0, 255),
            });

            var bytesLeft = texture.Buffer.Length;
            while (bytesLeft > 0)
            {
                var bytesToSend = Math.Min(bytesLeft, 512);
                var startIndex = texture.Buffer.Length - bytesLeft;
                var endIndex = startIndex + bytesToSend;

                await _gpu.SendFireAndForgetAsync(new AppendTexturePixelsOperation
                {
                    TextureId = textureId,
                    Pixels = texture.Buffer.AsMemory(startIndex, endIndex),
                });
                
                bytesLeft -= bytesToSend;
            }
        }
    }

    private static BufferRgb565 SubTexture(BufferRgb565 spriteSheet, int x, int y, int width, int height)
    {
        var imageBuffer = new BufferRgb565(width, height);
        imageBuffer.WriteBuffer(x, y, spriteSheet);

        return imageBuffer;
    }
    
    private static BufferRgb565 LoadTexture()
    {
        var image = Image.LoadFromFile(Path.Combine(AppContext.BaseDirectory, "spritesheet.bmp"));
        var imageBuffer = new BufferRgb565(0, 0);
        imageBuffer.WriteBuffer(0, 0, image.DisplayBuffer);

        return imageBuffer;
    }
}