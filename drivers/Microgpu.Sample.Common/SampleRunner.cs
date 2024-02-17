using System.Diagnostics;
using Microgpu.Common;
using Microgpu.Common.Operations;
using Microgpu.Sample.Common.Samples;

namespace Microgpu.Sample.Common;

public class SampleRunner
{
    private readonly CancellationToken _cancellationToken;
    private readonly long[] _frameTimes = new long[1000];
    private readonly Gpu _gpu;
    private readonly BatchOperation _gpuBatch = new();
    private readonly TimeSpan _minTimeBetweenFrames;
    private readonly BouncingTexture _bouncingTexture1;
    private readonly BouncingTexture _bouncingTexture2;
    private readonly FramerateDisplay _framerateDisplay = new();
    private readonly TextureManager _textureManager;
    private int _frameTimeIndex;
    
    public Octahedron Octahedron { get; }
    
    public SampleRunner(Gpu gpu,
        TimeSpan minTimeBetweenFrames,
        CancellationToken? cancellationToken = null)
    {
        _gpu = gpu ?? throw new ArgumentNullException(nameof(gpu));
        _textureManager = new TextureManager(_gpu);
        _minTimeBetweenFrames = minTimeBetweenFrames;
        _cancellationToken = cancellationToken ?? CancellationToken.None;
        Octahedron = new Octahedron(gpu);
        _bouncingTexture1 = new BouncingTexture(_gpu, _textureManager.Textures[0]);
        _bouncingTexture2 = new BouncingTexture(_gpu, _textureManager.Textures[1]);
    }

    public async Task Run()
    {
        Console.WriteLine($"Expecting API version: {Gpu.ValidApiVersionId}");
        Console.WriteLine();
        Console.WriteLine("GPU status:");
        Console.WriteLine($"Is initialized: {_gpu.IsInitialized}");
        Console.WriteLine($"Display resolution: {_gpu.DisplayResolution.X} x {_gpu.DisplayResolution.Y}");
        Console.WriteLine(
            $"Framebuffer resolution: {_gpu.FrameBufferResolution?.X ?? 0} x {_gpu.FrameBufferResolution?.Y ?? 0}");
        Console.WriteLine($"Color mode: {_gpu.ColorMode}");

        await _textureManager.SendTexturesToGpuAsync();
        
        var timeSinceLastFrame = Stopwatch.StartNew();
        while (!_cancellationToken.IsCancellationRequested)
        {
            var frameTime = timeSinceLastFrame.ElapsedMilliseconds;
            timeSinceLastFrame.Restart();
            _frameTimes[_frameTimeIndex] = frameTime;
            _frameTimeIndex++;
            if (_frameTimeIndex >= _frameTimes.Length)
            {
                _frameTimeIndex = 0;
                var average = _frameTimes.Average();
                Console.WriteLine($"Average frame time: {average}ms ({_frameTimes.Length / average:0}fps)");
            }


            var innerFrameTime = Stopwatch.StartNew();
            await ExecuteFrameLogic(TimeSpan.FromMilliseconds(frameTime));
            innerFrameTime.Stop();

            var waitTime = _minTimeBetweenFrames - innerFrameTime.Elapsed;
            if (waitTime < TimeSpan.FromMilliseconds(1))
                // Always wait 1 millisecond in case this is running on a low core count
                // system, so other tasks get a chance to run.
                waitTime = TimeSpan.FromMilliseconds(1);

            await Task.Delay(waitTime, _cancellationToken);
        }
    }

    private async Task ExecuteFrameLogic(TimeSpan frameTime)
    {
        _bouncingTexture1.RunNextFrame(frameTime, _gpuBatch);
        Octahedron.RunNextFrame(frameTime, _gpuBatch);
        _bouncingTexture2.RunNextFrame(frameTime, _gpuBatch);
        _framerateDisplay.RunNextFrame(frameTime, _gpuBatch);

        _gpuBatch.AddOperation(new PresentFramebufferOperation());
        await _gpu.SendFireAndForgetAsync(_gpuBatch);
    }
}