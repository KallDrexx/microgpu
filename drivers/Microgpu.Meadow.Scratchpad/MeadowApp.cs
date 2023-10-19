using System;
using System.Threading.Tasks;
using Meadow;
using Meadow.Devices;
using Meadow.Hardware;
using Meadow.Units;
using Microgpu.Common;
using Microgpu.Common.Comms;
using Microgpu.Sample.Common;

namespace Microgpu.Meadow.Scratchpad;

public class MeadowApp : App<F7FeatherV2>
{
    private Gpu _gpu = null!;

    public override async Task Initialize()
    {
        var reset = Device.CreateDigitalOutputPort(Device.Pins.D02, true);
        var handshake = Device.CreateDigitalInputPort(Device.Pins.D03);
        var chipSelect = Device.CreateDigitalOutputPort(Device.Pins.D04, true);

        var config = new SpiClockConfiguration(
            new Frequency(10, Frequency.UnitType.Megahertz),
            SpiClockConfiguration.Mode.Mode0);

        var spiBus = Device.CreateSpiBus(Device.Pins.SCK, Device.Pins.COPI, Device.Pins.CIPO, config);

        Console.WriteLine("Initializing GPU");
        var gpuCommunication = new MeadowSpiGpuCommunication(spiBus, handshake, reset, chipSelect);
        _gpu = await Gpu.CreateAsync(gpuCommunication);
        await _gpu.InitializeAsync(1);
    }

    public override async Task Run()
    {
        var sampleRunner = new SampleRunner(_gpu, TimeSpan.FromMilliseconds(0));
        await sampleRunner.Run();
    }
}