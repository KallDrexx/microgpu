using System;
using System.Threading.Tasks;
using Meadow;
using Meadow.Devices;
using Meadow.Foundation.ICs.IOExpanders;
using Meadow.Hardware;
using Meadow.Units;
using Microgpu.Common;
using Microgpu.Common.Comms;
using Microgpu.Sample.Common;

namespace Microgpu.Meadow.Scratchpad
{
    public class MeadowApp : App<F7FeatherV2>
    {
        private Gpu _gpu = null!;
        
        public override async Task Initialize()
        {
            var mcpIn = Device.CreateDigitalInterruptPort(
                Device.Pins.D10,
                InterruptMode.EdgeRising,
                ResistorMode.Disabled);
            var mcp = new Mcp23008(Device.CreateI2cBus(), 0x21, mcpIn);
            
            var reset = Device.CreateDigitalOutputPort(Device.Pins.D03, true);
            var handshake = mcp.CreateDigitalInputPort(mcp.Pins.GP6, ResistorMode.Disabled);
            // var handshake = mcp.CreateDigitalInterruptPort(mcp.Pins.GP6);
            var chipSelect = mcp.CreateDigitalOutputPort(mcp.Pins.GP5, true);
            
            // handshake.Changed += (sender, args) =>
            // {
            //     Console.WriteLine($"Handshake changed: {args.New}");
            // };


            // var projectLab = ProjectLab.Create();
            // var handshake = projectLab.MikroBus1.Pins.INT.CreateDigitalInputPort();
            // var reset = projectLab.MikroBus1.Pins.RST.CreateDigitalOutputPort(true);
            // var chipSelect = projectLab.MikroBus1.Pins.CS.CreateDigitalOutputPort(true);
            
            
            var config = new SpiClockConfiguration(
                new Frequency(10, Frequency.UnitType.Megahertz),
                SpiClockConfiguration.Mode.Mode0);
            
            // var spiBus = projectLab.SpiBus;
            // spiBus.Configuration.Speed = new Frequency(10, Frequency.UnitType.Megahertz);
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
            // while (true)
            // {
            //     await Task.Delay(100);
            // }
        }
    }
}