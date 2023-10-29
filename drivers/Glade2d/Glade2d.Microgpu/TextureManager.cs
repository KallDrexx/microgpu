using System;
using System.Collections.Generic;
using System.IO;
using System.Threading.Tasks;
using Glade2d.Graphics;
using Glade2d.Services;
using Meadow.Foundation.Graphics;
using Meadow.Foundation.Graphics.Buffers;
using Microgpu.Common;
using Microgpu.Common.Operations;

namespace Glade2d.Microgpu;

internal class TextureManager
{
    public record TextureInfo(byte TextureId, ushort Width, ushort Height);
    private record Texture(string Name, BufferRgb565 Buffer);
   
    private readonly Gpu _gpu;
    private readonly Dictionary<byte, Texture> _textures = new();
    private readonly Dictionary<string, BufferRgb565> _spriteSheets = new();
    private readonly Dictionary<string, byte> _textureLookup = new();
    private readonly string _contentRoot;
    private readonly Queue<byte> _addedTextures = new();
    private readonly Queue<byte> _removedTextures = new();
    
    public TextureManager(string contentRoot, Gpu gpu)
    {
        _contentRoot = contentRoot;
        _gpu = gpu;
    }
    
    public void Load(string textureName)
    {
        if (_textureLookup.ContainsKey(textureName))
        {
            return;
        }

        var buffer = LoadBitmapFile(textureName);
        var texture = new Texture(textureName, buffer);
        var textureId = GetNextFreeTextureId();
        _textures.Add(textureId, texture);
        _textureLookup.Add(textureName, textureId);
        _addedTextures.Enqueue(textureId);
    }

    public void Unload(string textureName)
    {
        if (!_textureLookup.ContainsKey(textureName))
        {
            return;
        }
        
        var id = _textureLookup[textureName];
        _textures.Remove(id);
        _textureLookup.Remove(textureName);
        _removedTextures.Enqueue(id);
    }

    public string LoadSubTexture(Frame frame)
    {
        var subTextureName = FormSubTextureName(frame);
        if (_textureLookup.ContainsKey(subTextureName))
        {
            return subTextureName;
        }
        
        if (!_spriteSheets.ContainsKey(frame.TextureName))
        {
            LoadSpriteSheet(frame.TextureName);
        }
        
        var buffer = _spriteSheets[frame.TextureName];
        var subBuffer = new BufferRgb565(frame.Width, frame.Height);
        for (var y = 0; y < frame.Height; y++)
        {
            for (var x = 0; x < frame.Width; x++)
            {
                var sourceIndex = (y + frame.Y) * (buffer.Width * 2) + ((x + frame.X) * 2);
                var destIndex = y * (frame.Width * 2) + (x * 2);
                subBuffer.Buffer[destIndex] = buffer.Buffer[sourceIndex];
                subBuffer.Buffer[destIndex + 1] = buffer.Buffer[sourceIndex + 1];
            }
        }
        
        var texture = new Texture(subTextureName, subBuffer);
        var textureId = GetNextFreeTextureId();
        _textures.Add(textureId, texture);
        _textureLookup.Add(subTextureName, textureId);
        _addedTextures.Enqueue(textureId);

        return subTextureName;
    }

    /// <summary>
    /// If there are texture changes, reset the textures on the GPU
    /// </summary>
    public async ValueTask ApplyTextureChanges()
    {
        while (_removedTextures.TryDequeue(out var textureId))
        {
            Console.WriteLine($"Removing texture {textureId} from the GPU");
            await _gpu.SendFireAndForgetAsync(new DefineTextureOperation<ColorRgb565>()
            {
                TextureId = textureId,
                Width = 0,
                Height = 0,
                TransparentColor = ColorRgb565.FromRgb888(0, 0, 0),
            });
        }

        while (_addedTextures.TryDequeue(out var textureId))
        {
            Console.WriteLine($"Adding texture {textureId} to the GPU");
            var texture = _textures[textureId];
            
            await _gpu.SendFireAndForgetAsync(new DefineTextureOperation<ColorRgb565>
            {
                TextureId = textureId,
                Width = (ushort)texture.Buffer.Width,
                Height = (ushort)texture.Buffer.Height,
                TransparentColor = ColorRgb565.FromRgb888(255, 0, 255)
            });

            var bytesLeft = texture.Buffer.Buffer.Length;
            while (bytesLeft > 0)
            {
                var bytesToSend = Math.Min(bytesLeft, 512);
                var startIndex = texture.Buffer.Buffer.Length - bytesLeft;

                await _gpu.SendFireAndForgetAsync(new AppendTexturePixelsOperation
                {
                    TextureId = textureId,
                    PixelBytes = texture.Buffer.Buffer.AsMemory(startIndex, bytesToSend)
                });

                bytesLeft -= bytesToSend;
            }
        }
    }
    
    public TextureInfo? GetTextureInfo(string textureName)
    {
        if (!_textureLookup.ContainsKey(textureName))
        {
            return null;
        }

        var textureId = _textureLookup[textureName];
        var texture = _textures[textureId];
        return new TextureInfo(textureId, (ushort) texture.Buffer.Width, (ushort) texture.Buffer.Height);
    }

    public TextureInfo? GetTextureInfo(Frame frame)
    {
        var textureName = FormSubTextureName(frame);
        return GetTextureInfo(textureName);
    }

    private static string FormSubTextureName(Frame frame)
    {
        var subTextureName = $"{frame.TextureName}___{frame.X}_{frame.Y}_{frame.Width}_{frame.Height}";
        return subTextureName;
    }

    private byte GetNextFreeTextureId()
    {
        for (byte x = 1; x < 231; x++)
        {
            if (!_textures.ContainsKey(x))
            {
                return x;
            }
        }

        throw new InvalidOperationException("All user defined texture slots are in use");
    }

    private void LoadSpriteSheet(string textureName)
    {
        if (_spriteSheets.ContainsKey(textureName))
        {
            return;
        }
        
        var buffer = LoadBitmapFile(textureName);
        _spriteSheets.Add(textureName, buffer);
    }
    
    private BufferRgb565 LoadBitmapFile(string name)
    {
        Console.WriteLine($"Attempting to LoadBitmapFile: {name}");
        var filePath = Path.Combine(_contentRoot, name);

        try
        {
            var img = Image.LoadFromFile(filePath);
            Console.WriteLine($"Color mode: {img.DisplayBuffer.ColorMode}");

            // Always make sure that the texture is formatted in the same color mode as the display
            var imgBuffer = new BufferRgb565(img.Width, img.Height);
            imgBuffer.WriteBuffer(0, 0, img.DisplayBuffer);
            LogService.Log.Trace($"{name} loaded to buffer of type {imgBuffer.GetType()}");
            return imgBuffer;
        }
        catch (Exception exception)
        {
            LogService.Log.Error($"Failed to load {filePath}: The file should be a 24bit bmp, in the root " +
                                 $"directory with BuildAction = Content, and Copy if Newer!",
                exception);
            
            throw;
        }
    }
    
}