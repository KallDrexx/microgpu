using System;
using System.Threading.Tasks;
using Meadow;
using Meadow.Devices;
using Meadow.Foundation.ICs.IOExpanders;
using Meadow.Foundation.Sensors.Buttons;
using Meadow.Hardware;
using Meadow.Units;
using Microgpu.Common.Comms;
using Microgpu.Common.Responses;
using Microgpu.Sample.Common;

namespace Microgpu.Meadow.Scratchpad
{
    public class MeadowApp : App<F7CoreComputeV2>
    {
        private Mcp23008 _mcp1 = null!;
        private Mcp23008 _mcp2 = null!;
        private IGpuCommunication _gpuCommunication = null!;
        
        public override Task Initialize()
        {
            var i2CBus = Device.CreateI2cBus();
            
            Console.WriteLine("Creating MCP1");
            var mcpReset = Device.CreateDigitalOutputPort(Device.Pins.D11, true);
            var mcp1Interrupt = Device.CreateDigitalInterruptPort(Device.Pins.D09, InterruptMode.EdgeRising);
            _mcp1 = new Mcp23008(i2CBus, 0x20, mcp1Interrupt, mcpReset);
            
            Console.WriteLine("Creating MCP2");
            var mcp2Interrupt = Device.CreateDigitalInterruptPort(Device.Pins.D10, InterruptMode.EdgeRising);
            _mcp2 = new Mcp23008(i2CBus, 0x21, mcp2Interrupt);
           
            Console.WriteLine("Initializing GPU");
            var chipSelectPort = _mcp1.CreateDigitalOutputPort(_mcp1.Pins.GP5, true);
            var resetPort = _mcp1.CreateDigitalOutputPort(_mcp1.Pins.GP7, true);
            var handshake = Device.CreateDigitalInputPort(Device.Pins.D05);
            
            var config = new SpiClockConfiguration(
                new Frequency(10, Frequency.UnitType.Megahertz),
                SpiClockConfiguration.Mode.Mode0);
        
            var spiBus =  Device.CreateSpiBus(Device.Pins.SPI5_SCK, Device.Pins.SPI5_COPI, Device.Pins.SPI5_CIPO, config); 
            //Device.CreateSpiBus(Device.Pins.SCK, Device.Pins.COPI, Device.Pins.CIPO, config);
            _gpuCommunication = new MeadowSpiGpuCommunication(spiBus, handshake, resetPort, chipSelectPort);

            return Task.CompletedTask;
        }

        public override async Task Run()
        {
            var dPadLeftPort = _mcp1.CreateDigitalInterruptPort(_mcp1.Pins.GP4, InterruptMode.EdgeBoth, ResistorMode.InternalPullUp);
            var dPadRightPort = _mcp1.CreateDigitalInterruptPort(_mcp1.Pins.GP2, InterruptMode.EdgeBoth, ResistorMode.InternalPullUp);
            var dPadLeft = new PushButton(dPadLeftPort);
            var dPadRight = new PushButton(dPadRightPort);
            
            var sampleRunner = new SampleRunner();
            dPadLeft.PressStarted += (_, _) => sampleRunner.LeftPressed = true;
            dPadLeft.PressEnded += (_, _) => sampleRunner.LeftPressed = false;
            dPadRight.PressStarted += (_, _) => sampleRunner.RightPressed = true;
            dPadLeft.PressEnded += (_, _) => sampleRunner.RightPressed = false;
            
            await sampleRunner.Run(new SampleRunner.SampleOptions
            {
                FramebufferScale = 1,
                GpuCommunication = _gpuCommunication,
            });
        }
    }
}