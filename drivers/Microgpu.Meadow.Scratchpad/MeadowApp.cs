using System;
using System.Threading.Tasks;
using Meadow;
using Meadow.Devices;
using Meadow.Units;
using Microgpu.Meadow.Common;

namespace Microgpu.Meadow.Scratchpad
{
    public class MeadowApp : App<F7FeatherV2>
    {
        private SpiGpuInterface _spiGpuInterface = null!;
        
        public override async Task Initialize()
        {
            var reset = Device.CreateDigitalOutputPort(Device.Pins.D02);
            var handshake = Device.CreateDigitalInputPort(Device.Pins.D03);
            var chipSelect = Device.CreateDigitalOutputPort(Device.Pins.D04);
            var spiBus = Device.CreateSpiBus(new Frequency(10, Frequency.UnitType.Megahertz));
            
            Console.WriteLine("Initializing GPU...");
            _spiGpuInterface = await SpiGpuInterface.CreateAsync(spiBus, handshake, reset, chipSelect);
            Console.WriteLine("GPU initialized");
        }
    }
}