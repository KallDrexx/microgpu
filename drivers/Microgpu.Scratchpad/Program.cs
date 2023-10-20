using Microgpu.Common;
using Microgpu.Common.Comms;
using Microgpu.Sample.Common;

Console.WriteLine("Connecting to TCP microgpu server on localhost:9123 ...");
var gpu = await Gpu.CreateAsync(new TcpGpuCommunication("localhost", 9123));
await gpu.InitializeAsync(1);

var sampleRunner = new SampleRunner(gpu, TimeSpan.FromMilliseconds(16));
await sampleRunner.Run();