using Glade2d;
using Glade2d.Graphics;
using Glade2d.Microgpu;
using Glade2d.Profiling;
using GladeSampleShared.Screens;
using Microgpu.Common.Comms;

Console.WriteLine("Connecting to TCP microgpu server on localhost:9123 ...");
var gpuCommunication = new TcpGpuCommunication("localhost", 9123);

var layerManager = new LayerManager();
var profiler = new Profiler();
var renderer = await MicrogpuRenderer.CreateAsync(gpuCommunication, layerManager, profiler, Environment.CurrentDirectory, 3);
var engine = new Game
{
    SleepMilliseconds = 16,
};

engine.Initialize(renderer, null, layerManager, profiler); 

await engine.Start(() => new GladeDemoScreen());