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
            LogStatus(status);

            if (!status.IsInitialized)
            {
                throw new InvalidOperationException("No initialization occurred");
            }
        }

        public override async Task Run()
        {
            var octahedron = new Octahedron();
            await octahedron.Run(_spiGpuInterface);

            // await _spiGpuInterface.SendFireAndForgetAsync(new DrawTriangleOperation<ColorRgb565>
            // {
            //     X0 = 60,
            //     Y0 = 20,
            //     X1 = 30,
            //     Y1 = 100,
            //     X2 = 90,
            //     Y2 = 100,
            //     Color = ColorRgb565.FromRgb888(255, 0, 0),
            // });
            //
            // await _spiGpuInterface.SendFireAndForgetAsync(new PresentFramebufferOperation());
            //
            // while (true)
            // {
            //     await Task.Delay(1000);
            // }
        }

        private static void LogStatus(StatusResponse status)
        {
            Console.WriteLine("Status received");
            Console.WriteLine($"Is initialized: {status.IsInitialized}");
            Console.WriteLine($"Display resolution: {status.DisplayWidth}x{status.DisplayHeight}");
            Console.WriteLine($"Framebuffer resolution: {status.FrameBufferWidth}x{status.FrameBufferHeight}");
            Console.WriteLine($"Color mode: {status.ColorMode}");
        }
    }
}