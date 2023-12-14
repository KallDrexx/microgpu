using System;
using System.Threading.Tasks;
using Glade2d.Graphics;
using Glade2d.Profiling;
using GladeSampleShared.Screens;
using Meadow;
using Meadow.Devices;
using Meadow.Foundation.ICs.IOExpanders;
using Meadow.Hardware;
using Meadow.Units;
using Microgpu.Common;
using Microgpu.Common.Comms;

namespace Glade2d.Microgpu.Sample.ProjectLab;

public class MeadowApp : App<F7CoreComputeV2>
{
    private Game _engine = null!;

    public override async Task Initialize()
    {
        var reset = Device.CreateDigitalOutputPort(Device.Pins.D02, true);
        var handshake = Device.CreateDigitalInputPort(Device.Pins.D03, ResistorMode.Disabled);
        var chipSelect = Device.CreateDigitalOutputPort(Device.Pins.D14, true);

        var config = new SpiClockConfiguration(
            new Frequency(10, Frequency.UnitType.Megahertz),
            SpiClockConfiguration.Mode.Mode0);

        var spiBus = Device.CreateSpiBus(Device.Pins.SPI5_SCK, Device.Pins.SPI5_COPI, Device.Pins.SPI5_CIPO, config);

        Console.WriteLine("Initializing GPU");
        var gpuCommunication = new MeadowSpiGpuCommunication(spiBus, handshake, reset, chipSelect);
        var layerManager = new LayerManager();
        var profiler = new Profiler();
        var renderer = await MicrogpuRenderer.CreateAsync(gpuCommunication, layerManager, profiler, MeadowOS.FileSystem.UserFileSystemRoot, 2);
        _engine = new Game();
        _engine.Initialize(renderer, null, layerManager, profiler); 
        _engine.Profiler.IsActive = true;
    }
    
    public override async Task Run()
    {
        await _engine.Start(() => new GladeDemoScreen());
    }
}