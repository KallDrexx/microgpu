﻿using System;
using System.Threading.Tasks;
using Meadow;
using Meadow.Devices;
using Meadow.Foundation.Sensors.Motion;
using Meadow.Hardware;
using Meadow.Units;
using Microgpu.Common;
using Microgpu.Common.Comms;
using Microgpu.Sample.Common;
using Microgpu.Sample.Common.Samples;

namespace Microgpu.Meadow.Scratchpad
{
    public class MeadowApp : App<F7CoreComputeV2>
    {
        private Gpu _gpu = null!;
        private Bmi270 _bmi270 = null!;
        private SampleRunner _sampleRunner = null!;

        public override async Task Initialize()
        {
            var reset = Device.CreateDigitalOutputPort(Device.Pins.D02, true);
            var handshake = Device.CreateDigitalInputPort(Device.Pins.D03, ResistorMode.InternalPullDown);
            var chipSelect = Device.CreateDigitalOutputPort(Device.Pins.D14, true);

            var config = new SpiClockConfiguration(
                new Frequency(10, Frequency.UnitType.Megahertz),
                SpiClockConfiguration.Mode.Mode0);

            var spiBus = Device.CreateSpiBus(Device.Pins.SPI5_SCK, Device.Pins.SPI5_COPI, Device.Pins.SPI5_CIPO,
                config);

            Console.WriteLine("Initializing GPU");
            var gpuCommunication = new MeadowSpiGpuCommunication(spiBus, handshake, reset, chipSelect);
            _gpu = await Gpu.CreateAsync(gpuCommunication);
            await _gpu.InitializeAsync(1);

            Console.WriteLine("Initializing BMI270");
            _bmi270 = new Bmi270(Device.CreateI2cBus());
            _bmi270.Updated += HandleBmi270Reading;
            _bmi270.StartUpdating(TimeSpan.FromMilliseconds(100));

            Console.WriteLine("Initializing sample runner");
            _sampleRunner = new SampleRunner(_gpu, TimeSpan.FromMilliseconds(0));
        }

        public override async Task Run()
        {
            Console.WriteLine("Starting sample");
            await _sampleRunner.Run();
        }

        private void HandleBmi270Reading(object sender,
            IChangeResult<(Acceleration3D? Acceleration3D,
                AngularVelocity3D? AngularVelocity3D,
                Temperature? Temperature)> result)
        {
            try
            {
                if (result?.New.AngularVelocity3D != null)
                    _sampleRunner.Octahedron.RotationDegreesPerSecond = new Octahedron.Vector3(
                        (float)result.New.AngularVelocity3D.Value.X.DegreesPerSecond * 2,
                        (float)result.New.AngularVelocity3D.Value.Y.DegreesPerSecond * 2,
                        (float)result.New.AngularVelocity3D.Value.Z.DegreesPerSecond * 2);
            }
            catch (NullReferenceException)
            {
            }
        }
    }
}