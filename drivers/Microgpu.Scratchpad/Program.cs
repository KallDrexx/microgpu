using Microgpu.Common.Comms;
using Microgpu.Sample.Common;

Console.WriteLine("Connecting to TCP microgpu server on localhost:9123 ...");
await SampleRunner.Run(new SampleRunner.SampleOptions
{
    GpuCommunication = new TcpGpuCommunication("localhost", 9123),
    FramebufferScale = 1,
    MinTimeBetweenFrames = TimeSpan.FromMilliseconds(16),
});
