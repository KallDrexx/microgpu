using System;
using System.Collections.Generic;
using System.Numerics;
using System.Threading.Tasks;
using Glade2d.Graphics;
using Glade2d.Services;
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
    private readonly TextureManager.TextureInfo _textureInfo;
    private readonly List<DrawCommand> _pendingDrawCommands = new();
    private Vector2 _internalOrigin = Vector2.Zero;
    private bool _clearPending = true;

    public Point CameraOffset { get; set; }
    public Color BackgroundColor { get; set; }
    public Color TransparentColor { get; set; } = Color.Magenta;
    public bool DrawLayerWithTransparency { get; set; } // Has no effect for microgpu
    public int Width { get; }
    public int Height { get; }
    public IFont DefaultFont { get; } = new Font4x6();

    public Layer(TextureManager textureManager, int width, int height)
    {
        _textureManager = textureManager;
        Width = width;
        Height = height;

        var textureName = _textureManager.CreateLayerTexture(width, height);
        _textureInfo = _textureManager.GetTextureInfo(textureName)
                       ?? throw new InvalidOperationException("Layer texture not created");
    }

    public void Clear()
    {
        _pendingDrawCommands.Clear();
        _clearPending = true;
    }

    public void DrawTexture(Frame frame, Point topLeftOnLayer)
    {
        var subTextureName = _textureManager.LoadSubTexture(frame);
        _pendingDrawCommands.Add(new DrawCommand(subTextureName, topLeftOnLayer));
    }

    public void DrawTexture(BufferRgb565 texture, Point topLeftOnTexture, Point topLeftOnLayer, Dimensions drawSize,
        bool ignoreTransparency = false)
    {
        throw new System.NotImplementedException();
    }

    public void Shift(Vector2 shiftAmount)
    {
        _internalOrigin -= shiftAmount;
        
        // Normalize the origin so it's always somewhere within the bounds
        // of the layer.
        while (_internalOrigin.X < 0) _internalOrigin.X += Width;
        while (_internalOrigin.X >= Width) _internalOrigin.X -= Width;
        while (_internalOrigin.Y < 0) _internalOrigin.Y += Height;
        while (_internalOrigin.Y >= Height) _internalOrigin.Y -= Height;
    }

    public void DrawText(Point position, string text, IFont? font = null, Color? color = null)
    {
        throw new System.NotImplementedException();
    }

    public async Task CreateDrawOperations(Gpu gpu)
    {
        await HandlePendingDrawCommands(gpu);
        await DrawLayerToFrameBuffer(gpu);
    }

    private async Task DrawLayerToFrameBuffer(Gpu gpu)
    {
        // Draw the layer to the frame buffer in 4 parts to handle the horizontal origin
        // being shifted around.
        var batch = new BatchOperation();
        
        // Bottom Right
        batch.AddOperation(new DrawTextureOperation
        {
            SourceTextureId = _textureInfo.TextureId,
            TargetTextureId = 0,
            SourceStartX = (ushort)_internalOrigin.X,
            SourceStartY = (ushort)_internalOrigin.Y,
            SourceWidth = (ushort)(Width - _internalOrigin.X),
            SourceHeight = (ushort)(Height - _internalOrigin.Y),
            TargetStartX = (short)CameraOffset.X,
            TargetStartY = (short)CameraOffset.Y,
            IgnoreTransparency = !DrawLayerWithTransparency,
        });
        
        // Bottom Left
        batch.AddOperation(new DrawTextureOperation
        {
            SourceTextureId = _textureInfo.TextureId,
            TargetTextureId = 0,
            SourceStartX = 0,
            SourceStartY = (ushort)_internalOrigin.Y,
            SourceWidth = (ushort)_internalOrigin.X,
            SourceHeight = (ushort)(Height - _internalOrigin.Y),
            TargetStartX = (short)(CameraOffset.X + (Width - _internalOrigin.X)),
            TargetStartY = (short)CameraOffset.Y,
            IgnoreTransparency = !DrawLayerWithTransparency,
        });
        
        // Top Right
        batch.AddOperation(new DrawTextureOperation
        {
            SourceTextureId = _textureInfo.TextureId,
            TargetTextureId = 0,
            SourceStartX = (ushort)_internalOrigin.X,
            SourceStartY = 0,
            SourceWidth = (ushort)(Width - _internalOrigin.X),
            SourceHeight = (ushort)_internalOrigin.Y,
            TargetStartX = (short)CameraOffset.X,
            TargetStartY = (short)(CameraOffset.Y + (Height - _internalOrigin.Y)),
            IgnoreTransparency = !DrawLayerWithTransparency,
        });
        
        // Top Left
        batch.AddOperation(new DrawTextureOperation
        {
            SourceTextureId = _textureInfo.TextureId,
            TargetTextureId = 0,
            SourceStartX = 0,
            SourceStartY = 0,
            SourceWidth = (ushort)_internalOrigin.X,
            SourceHeight = (ushort)_internalOrigin.Y,
            TargetStartX = (short)(CameraOffset.X + (Width - _internalOrigin.X)),
            TargetStartY = (short)(CameraOffset.Y + (Height - _internalOrigin.Y)),
            IgnoreTransparency = !DrawLayerWithTransparency,
        });
        
        await gpu.SendFireAndForgetAsync(batch);
    }
    
    private async Task HandlePendingDrawCommands(Gpu gpu)
    {
        if (_clearPending)
        {
            await gpu.SendFireAndForgetAsync(new DrawRectangleOperation<ColorRgb565>
            {
                TextureId = _textureInfo.TextureId,
                StartX = 0,
                StartY = 0,
                Width = (ushort)Width,
                Height = (ushort)Height,
                Color = BackgroundColor.ToColorRgb565(),
            });

            _clearPending = false;
        }

        var batch = new BatchOperation();
        foreach (var drawCommand in _pendingDrawCommands)
        {
            var drawingTexture = _textureManager.GetTextureInfo(drawCommand.TextureName);
            if (drawingTexture == null)
            {
                var message = $"Texture {drawCommand.TextureName} not loaded";
                throw new InvalidOperationException(message);
            }

            var operation = new DrawTextureOperation
            {
                SourceTextureId = drawingTexture.TextureId,
                TargetTextureId = _textureInfo.TextureId,
                SourceStartX = 0,
                SourceStartY = 0,
                SourceWidth = drawingTexture.Width,
                SourceHeight = drawingTexture.Height,
                TargetStartX = (short)drawCommand.TopLeftOnLayer.X,
                TargetStartY = (short)drawCommand.TopLeftOnLayer.Y,
                IgnoreTransparency = false,
            };
            
            if (!batch.AddOperation(operation))
            {
                await gpu.SendFireAndForgetAsync(batch);
                batch.AddOperation(operation);
            }
        }

        if (batch.HasAnyOperations())
        {
            await gpu.SendFireAndForgetAsync(batch);
        }

        _pendingDrawCommands.Clear();
    }
}