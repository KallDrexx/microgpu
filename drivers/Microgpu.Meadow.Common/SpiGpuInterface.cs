using System;
using System.Threading.Tasks;
using Meadow.Hardware;
using Microgpu.Common.Operations;

namespace Microgpu.Meadow.Common
{
    public class SpiGpuInterface
    {
        private readonly ISpiBus _spiBus;
        private readonly IDigitalInputPort _handshakePin;
        private readonly IDigitalOutputPort _resetPin;
        private readonly IDigitalOutputPort _chipSelectPin;
        private readonly byte[] _writeBuffer = new byte[1024];

        private SpiGpuInterface(ISpiBus spiBus, 
            IDigitalInputPort handshakePin, 
            IDigitalOutputPort resetPin, 
            IDigitalOutputPort chipSelectPin)
        {
            _spiBus = spiBus;
            _handshakePin = handshakePin;
            _resetPin = resetPin;
            _chipSelectPin = chipSelectPin;
        }
        
        public static async Task<SpiGpuInterface> CreateAsync(ISpiBus spiBus, 
            IDigitalInputPort handshakePin, 
            IDigitalOutputPort resetPin, 
            IDigitalOutputPort chipSelectPin)
        {
            var spi = new SpiGpuInterface(spiBus, handshakePin, resetPin, chipSelectPin);
            await spi.ResetAsync();

            return spi;
        }

        public void SendFireAndForget(IFireAndForgetOperation operation)
        {
            var byteCount = operation.Serialize(_writeBuffer);
            _spiBus.Write(_chipSelectPin, _writeBuffer.AsSpan(0, byteCount));
        }

        private async Task ResetAsync()
        {
            // Ensure chip select starts high/inactive
            _chipSelectPin.State = true;
            
            // Reset the display
            _resetPin.State = false;
            await Task.Delay(100);
            _resetPin.State = true;
            
            // Wait for the gpu to be ready
            var startedAt = DateTime.Now;
            while (!_handshakePin.State)
            {
                if (DateTime.Now - startedAt > TimeSpan.FromSeconds(5))
                {
                    throw new TimeoutException("Timeout waiting for gpu to start");
                }
                
                await Task.Delay(100);
            }
        }
    }
}