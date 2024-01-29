using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Glade2d.Graphics;
using Glade2d.Profiling;
using Meadow.Foundation;
using Microgpu.Common;
using Microgpu.Common.Comms;
using Microgpu.Common.Operations;

namespace Glade2d.Microgpu;

public class MicrogpuRenderer : IRenderer
{
    private readonly Gpu _gpu;
    private readonly TextureManager _textureManager;
    private readonly LayerManager _layerManager;
    private readonly BatchOperation _activeBatch = new();
    private readonly Profiler _profiler;

    public bool ShowPerf { get; set; }
    public int Width => (int)_gpu.FrameBufferResolution!.Value.X;
    public int Height => (int)_gpu.FrameBufferResolution!.Value.Y;

    public Color BackgroundColor { get; set; } = Color.Black;

    private MicrogpuRenderer(Gpu gpu, TextureManager textureManager, LayerManager layerManager, Profiler profiler)
    {
        _gpu = gpu;
        _textureManager = textureManager;
        _layerManager = layerManager;
        _profiler = profiler;
    }

    public static async Task<MicrogpuRenderer> CreateAsync(
        IGpuCommunication gpuCommunication,
        LayerManager layerManager,
        Profiler profiler,
        string? contentRoot = null,
        byte frameBufferScale = 1)
    {
        var gpu = await Gpu.CreateAsync(gpuCommunication);
        await gpu.InitializeAsync(frameBufferScale);
        var status = await gpu.SendResponsiveOperationAsync(new GetStatusOperation());
        if (!status.IsInitialized)
        {
            throw new InvalidOperationException("GPU not initialized");
        }
       
        var textureManager = new TextureManager(contentRoot ?? Environment.CurrentDirectory, gpu);
        return new MicrogpuRenderer(gpu, textureManager, layerManager, profiler);
    }

    public async ValueTask RenderAsync(List<Sprite> sprites)
    {
        _profiler.StartTiming("Microgpu.LoadSubTextures");
        // TODO: No way to know if sprites should be unloaded.  Hole in Glade2d API atm
        // This also means every time a new sprite texture is used, we have to completely 
        // reload all textures.  Glade2d needs a bette way to preload textures.
        foreach (var sprite in sprites)
        {
            _textureManager.LoadSubTexture(sprite.CurrentFrame);
        }
        _profiler.StopTiming("Microgpu.LoadSubTextures");

        _profiler.StartTiming("Microgpu.ApplyTextureChanges");
        await _textureManager.ApplyTextureChanges();
        _profiler.StopTiming("Microgpu.ApplyTextureChanges");
        
        // Clear the screen
        await _gpu.SendFireAndForgetAsync(new DrawRectangleOperation<ColorRgb565>
        {
            TextureId = 0,
            StartX = 0,
            StartY = 0,
            Width = (ushort)Width,
            Height = (ushort)Height,
            Color = BackgroundColor.ToColorRgb565(),
        });
       
        _profiler.StartTiming("Microgpu.BackgroundLayers");
        var backgroundLayerEnumerator = _layerManager.BackgroundLayerEnumerator();
        while (backgroundLayerEnumerator.MoveNext())
        {
            if (backgroundLayerEnumerator.Current != null)
            {
                var layer = (Layer)backgroundLayerEnumerator.Current;
                await layer.CreateDrawOperations(_gpu);
            }
        }
        _profiler.StopTiming("Microgpu.BackgroundLayers");

        _profiler.StartTiming("Microgpu.Sprites");
        foreach (var sprite in sprites)
        {
            var textureInfo = _textureManager.GetTextureInfo(sprite.CurrentFrame);
            if (textureInfo == null)
            {
                var message = $"Texture {sprite.CurrentFrame.TextureName} not loaded";
                throw new InvalidOperationException(message);
            }

            var operation = new DrawTextureOperation
            {
                SourceTextureId = textureInfo.TextureId,
                TargetTextureId = 0,
                SourceStartX = 0,
                SourceStartY = 0,
                SourceWidth = textureInfo.Width,
                SourceHeight = textureInfo.Height,
                TargetStartX = (short)sprite.X,
                TargetStartY = (short)sprite.Y,
            };
            
            var wasAdded = _activeBatch.AddOperation(operation);
            if (!wasAdded)
            {
                await _gpu.SendFireAndForgetAsync(_activeBatch);
                _activeBatch.AddOperation(operation);
            }
        }

        if (_activeBatch.HasAnyOperations())
        {
            await _gpu.SendFireAndForgetAsync(_activeBatch);
        }

        _profiler.StopTiming("Microgpu.Sprites");

        _profiler.StopTiming("Microgpu.ForegroundLayers");
        var foregroundLayerEnumerator = _layerManager.ForegroundLayerEnumerator();
        while (foregroundLayerEnumerator.MoveNext())
        {
            if (foregroundLayerEnumerator.Current != null)
            {
                var layer = (Layer)foregroundLayerEnumerator.Current;
                await layer.CreateDrawOperations(_gpu);
            }
        }
        _profiler.StopTiming("Microgpu.ForegroundLayers");
        
        await _gpu.SendFireAndForgetAsync(new PresentFramebufferOperation());
    }

    public ILayer CreateLayer(Dimensions dimensions)
    {
        return new Layer(_textureManager, dimensions.Width, dimensions.Height);
    }
}