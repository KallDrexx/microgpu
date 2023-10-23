using System;
using System.Collections.Generic;
using System.Numerics;
using System.Threading.Tasks;
using Glade2d.Graphics;
using Meadow.Foundation;
using Meadow.Foundation.Graphics;
using Meadow.Foundation.Graphics.Buffers;
using Microgpu.Common;
using Microgpu.Common.Operations;

namespace Glade2d.Microgpu;

internal class Layer : ILayer
{
    private readonly record struct DrawCommand(string TextureName, Point TopLeftOnLayer);
    private readonly TextureManager _textureManager;
    private readonly List<DrawCommand> _drawCommands = new();
    private Vector2 _shiftAmount;

    public Point CameraOffset { get; set; }
    public Color BackgroundColor { get; set; }
    public Color TransparentColor { get; set; } = Color.Magenta;
    public bool DrawLayerWithTransparency { get; set; } // Has no effect for microgpu
    public int Width { get; private set; }
    public int Height { get; private set; }
    public IFont DefaultFont { get; set; } = new Font4x6();

    public Layer(TextureManager textureManager, int width, int height)
    {
        _textureManager = textureManager;
        Width = width;
        Height = height;
    }

    public void Clear()
    {
        _drawCommands.Clear();
    }

    public void DrawTexture(Frame frame, Point topLeftOnLayer)
    {
        var subTextureName = _textureManager.LoadSubTexture(frame);
        _drawCommands.Add(new DrawCommand(subTextureName, topLeftOnLayer));
    }

    public void DrawTexture(BufferRgb565 texture, Point topLeftOnTexture, Point topLeftOnLayer, Dimensions drawSize,
        bool ignoreTransparency = false)
    {
        throw new System.NotImplementedException();
    }

    public void Shift(Vector2 shiftAmount)
    {
        _shiftAmount += shiftAmount;
        while (_shiftAmount.X >= Width)
        {
            _shiftAmount.X -= Width;
        }

        while (_shiftAmount.X <= -Width)
        {
            _shiftAmount.X += Width;
        }

        while (_shiftAmount.Y >= Height)
        {
            _shiftAmount.Y -= Height;
        }

        while (_shiftAmount.Y <= -Height)
        {
            _shiftAmount.Y += Height;
        }
    }

    public void DrawText(Point position, string text, IFont? font = null, Color? color = null)
    {
        throw new System.NotImplementedException();
    }

    public async Task CreateDrawOperations(Gpu gpu)
    {
        var startX = Math.Max(CameraOffset.X, 0);
        var startY = Math.Max(CameraOffset.Y, 0);
        var width = Width - (startX - CameraOffset.X);
        var height = Height - (startY - CameraOffset.Y);
        
        await gpu.SendFireAndForgetAsync(new DrawRectangleOperation<ColorRgb565>
        {
            StartX = (ushort)startX,
            StartY = (ushort)startY,
            Width = (ushort)width,
            Height = (ushort)height,
            Color = BackgroundColor.ToColorRgb565(),
        });

        var batch = new BatchOperation();
        var countInBatch = 0;
        foreach (var drawCommand in _drawCommands)
        {
            var textureId = _textureManager.GetTextureId(drawCommand.TextureName);
            if (textureId == null)
            {
                var message = $"Texture {drawCommand.TextureName} not loaded";
                throw new InvalidOperationException(message);
            }

            var shiftedX = drawCommand.TopLeftOnLayer.X + _shiftAmount.X;
            var shiftedY = drawCommand.TopLeftOnLayer.Y + _shiftAmount.Y;
            if (shiftedX < 0)
            {
                shiftedX += Width;
            }

            if (shiftedX >= Width)
            {
                shiftedX -= Width;
            }

            if (shiftedY < 0)
            {
                shiftedY += Height;
            }

            if (shiftedY >= Height)
            {
                shiftedY -= Height;
            }
            
            batch.AddOperation(new DrawTextureOperation
            {
                TextureId = (byte)textureId.Value,
                X = (short)(shiftedX + CameraOffset.X),
                Y = (short)(shiftedY + CameraOffset.Y),
            });

            countInBatch++;
            if (countInBatch > 100)
            {
                await gpu.SendFireAndForgetAsync(batch);
                countInBatch = 0;
            }
        }

        if (countInBatch > 0)
        {
            await gpu.SendFireAndForgetAsync(batch);
        }
    }
}