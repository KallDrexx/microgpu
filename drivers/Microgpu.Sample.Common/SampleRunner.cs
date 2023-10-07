using System.Diagnostics;
using System.Numerics;
using Microgpu.Common;
using Microgpu.Common.Comms;

namespace Microgpu.Sample.Common;

public class SampleRunner
{
    public bool RightPressed { get; set; }
    public bool LeftPressed { get; set; }
    
    public async Task Run(SampleOptions options)
    {
        var frameTimes = new long[1000];
        var frameTimeIndex = 0;
        
        if (options == null)
        {
            throw new ArgumentNullException(nameof(options));
        }

        if (options.GpuCommunication == null)
        {
            throw new ArgumentNullException(nameof(options.GpuCommunication));
        }

        var gpu = await Gpu.CreateAsync(options.GpuCommunication);
        await gpu.InitializeAsync(options.FramebufferScale);

        Console.WriteLine("GPU status:");
        Console.WriteLine($"Is initialized: {gpu.IsInitialized}");
        Console.WriteLine($"Display resolution: {gpu.DisplayResolution.X} x {gpu.DisplayResolution.Y}");
        Console.WriteLine(
            $"Framebuffer resolution: {gpu.FrameBufferResolution?.X ?? 0} x {gpu.FrameBufferResolution?.Y ?? 0}");
        Console.WriteLine($"Color mode: {gpu.ColorMode}");

        var octahedron = new Octahedron(gpu);
        var timeSinceLastFrame = Stopwatch.StartNew();
        while (!options.CancellationToken.IsCancellationRequested)
        {
            var frameTime = timeSinceLastFrame.ElapsedMilliseconds;
            timeSinceLastFrame.Restart();
            frameTimes[frameTimeIndex] = frameTime;
            frameTimeIndex++;
            if (frameTimeIndex >= frameTimes.Length)
            {
                frameTimeIndex = 0;
                var average = frameTimes.Average();
                Console.WriteLine($"Average frame time: {average}ms ({frameTimes.Length / average:0}fps)");
            }
            
            var innerFrameTime = Stopwatch.StartNew();
            await octahedron.RunNextFrame(TimeSpan.FromMilliseconds(frameTime), RightPressed, LeftPressed);
            innerFrameTime.Stop();
            
            var waitTime = options.MinTimeBetweenFrames - innerFrameTime.Elapsed;
            if (waitTime < TimeSpan.FromMilliseconds(1))
            {
                // Always wait 1 millisecond in case this is running on a low core count
                // system, so other tasks get a chance to run.
                waitTime = TimeSpan.FromMilliseconds(1);
            }

            await Task.Delay(waitTime);
        }
    }

    public class SampleOptions
    {
        public IGpuCommunication? GpuCommunication { get; set; }
        public TimeSpan MinTimeBetweenFrames { get; set; }
        public byte FramebufferScale { get; set; } = 1;
        public CancellationToken CancellationToken { get; set; } = CancellationToken.None;
    }
}