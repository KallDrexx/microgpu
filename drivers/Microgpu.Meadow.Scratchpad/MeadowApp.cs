using System;
using System.Threading.Tasks;
using Meadow;
using Meadow.Devices;
using Meadow.Hardware;
using Meadow.Units;
using Microgpu.Common.Comms;
using Microgpu.Common.Responses;
using Microgpu.Sample.Common;

namespace Microgpu.Meadow.Scratchpad
{
    public class MeadowApp : App<F7FeatherV2>
    {
        private IGpuCommunication _gpuCommunication = null!;
        
        public override Task Initialize()
        {
            var reset = Device.CreateDigitalOutputPort(Device.Pins.D02, true);
            var handshake = Device.CreateDigitalInputPort(Device.Pins.D03);
            var chipSelect = Device.CreateDigitalOutputPort(Device.Pins.D04, true);
            
            var config = new SpiClockConfiguration(
                new Frequency(10, Frequency.UnitType.Megahertz),
                SpiClockConfiguration.Mode.Mode0);
        
            var spiBus = Device.CreateSpiBus(Device.Pins.SCK, Device.Pins.COPI, Device.Pins.CIPO, config);
            _gpuCommunication = new MeadowSpiGpuCommunication(spiBus, handshake, reset, chipSelect);

            return Task.CompletedTask;
        }

        public override async Task Run()
        {
            await SampleRunner.Run(new SampleRunner.SampleOptions
            {
                FramebufferScale = 1,
                GpuCommunication = _gpuCommunication,
            });
        }
    }
}