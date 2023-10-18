using System.Diagnostics;
using System.Numerics;
using Microgpu.Common;
using Microgpu.Common.Comms;
using Microgpu.Common.Operations;

namespace Microgpu.Sample.Common;

public class SampleRunner
{
    private readonly Gpu _gpu;
    private readonly TimeSpan _minTimeBetweenFrames;
    private readonly CancellationToken _cancellationToken;
    private readonly BatchOperation _gpuBatch = new();
    private readonly Octahedron _octahedron;
    private readonly BouncingTexture _bouncingTexture;
    private readonly long[] _frameTimes = new long[1000];
    private int _frameTimeIndex;
    
    public SampleRunner(Gpu gpu, 
        TimeSpan minTimeBetweenFrames, 
        CancellationToken? cancellationToken = null)
    {
        _gpu = gpu ?? throw new ArgumentNullException(nameof(gpu));
        _minTimeBetweenFrames = minTimeBetweenFrames;
        _cancellationToken = cancellationToken ?? CancellationToken.None;
        _octahedron = new Octahedron(gpu);
        _bouncingTexture = new BouncingTexture(_gpu, 0);
    }
    
    public async Task Run()
    {
        Console.WriteLine("GPU status:");
        Console.WriteLine($"Is initialized: {_gpu.IsInitialized}");
        Console.WriteLine($"Display resolution: {_gpu.DisplayResolution.X} x {_gpu.DisplayResolution.Y}");
        Console.WriteLine(
            $"Framebuffer resolution: {_gpu.FrameBufferResolution?.X ?? 0} x {_gpu.FrameBufferResolution?.Y ?? 0}");
        Console.WriteLine($"Color mode: {_gpu.ColorMode}");

        var textureManager = new TextureManager(_gpu);
        await textureManager.SendTexturesToGpuAsync();
        
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
            {
                // Always wait 1 millisecond in case this is running on a low core count
                // system, so other tasks get a chance to run.
                waitTime = TimeSpan.FromMilliseconds(1);
            }

            await Task.Delay(waitTime, _cancellationToken);
        }
    }

    private async Task ExecuteFrameLogic(TimeSpan frameTime)
    {
        await _bouncingTexture.RunNextFrameAsync(frameTime);
        // _octahedron.RunNextFrame(frameTime, _gpuBatch);
       
        _gpuBatch.AddOperation(new PresentFramebufferOperation());
        await _gpu.SendFireAndForgetAsync(_gpuBatch);
    }
}