using System;
using System.Threading.Tasks;
using Meadow;
using Meadow.Devices;
using Meadow.Hardware;
using Meadow.Units;
using Microgpu.Common;
using Microgpu.Common.Operations;
using Microgpu.Common.Responses;
using Microgpu.Meadow.Common;

namespace Microgpu.Meadow.Scratchpad
{
    public class MeadowApp : App<F7FeatherV2>
    {
        private SpiGpuInterface _spiGpuInterface = null!;
        private int _width, _height;
        
        public override async Task Initialize()
        {
            var reset = Device.CreateDigitalOutputPort(Device.Pins.D02, true);
            var handshake = Device.CreateDigitalInputPort(Device.Pins.D03);
            var chipSelect = Device.CreateDigitalOutputPort(Device.Pins.D04, true);
            
            var config = new SpiClockConfiguration(
                new Frequency(10, Frequency.UnitType.Megahertz),
                SpiClockConfiguration.Mode.Mode0);
        
            var spiBus = Device.CreateSpiBus(Device.Pins.SCK, Device.Pins.COPI, Device.Pins.CIPO, config);
            
            Console.WriteLine("Resetting the GPU...");
            _spiGpuInterface = await SpiGpuInterface.CreateAsync(spiBus, handshake, reset, chipSelect);
            
            Console.WriteLine("GPU reset");

            var status = await _spiGpuInterface.SendResponsiveOperationAsync(new GetStatusOperation());
            LogStatus(status);

            Console.WriteLine("Sending initialization command...");
            await _spiGpuInterface.SendFireAndForgetAsync(new InitializeOperation
            {
                FrameBufferScale = 1
            });
            
            status = await _spiGpuInterface.SendResponsiveOperationAsync(new GetStatusOperation());
            _width = status.FrameBufferWidth;
            _height = status.FrameBufferHeight;
            LogStatus(status);

            if (!status.IsInitialized)
            {
                throw new InvalidOperationException("No initialization occurred");
            }
        }

        public override async Task Run()
        {
            var octahedron = new Octahedron();
            await octahedron.Run(_spiGpuInterface, _width, _height);
        }

        private static void LogStatus(StatusResponse status)
        {
            Console.WriteLine("Status received");
            Console.WriteLine($"Is initialized: {status.IsInitialized}");
            Console.WriteLine($"Display resolution: {status.DisplayWidth}x{status.DisplayHeight}");
            Console.WriteLine($"Framebuffer resolution: {status.FrameBufferWidth}x{status.FrameBufferHeight}");
            Console.WriteLine($"Color mode: {status.ColorMode}");
            Console.WriteLine($"Max Bytes: {status.MaxBytes}");
        }
    }
}