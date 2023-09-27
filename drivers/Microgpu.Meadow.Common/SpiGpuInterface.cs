using System;
using System.Diagnostics;
using System.Linq;
using System.Threading.Tasks;
using Meadow.Hardware;
using Microgpu.Common.Operations;
using Microgpu.Common.Responses;

namespace Microgpu.Meadow.Common
{
    public class SpiGpuInterface
    {
        private readonly ISpiBus _spiBus;
        private readonly IDigitalInputPort _handshakePin;
        private readonly IDigitalOutputPort _resetPin;
        private readonly IDigitalOutputPort _chipSelectPin;
        private readonly byte[] _writeBuffer = new byte[1024];
        private readonly byte[] _readBuffer = new byte[1024];

        private SpiGpuInterface(ISpiBus spiBus, 
            IDigitalInputPort handshakePin, 
            IDigitalOutputPort resetPin, 
            IDigitalOutputPort chipSelectPin)
        {
            _spiBus = spiBus;
            _handshakePin = handshakePin;
            _resetPin = resetPin;
            _chipSelectPin = chipSelectPin;

            _resetPin.State = true;
            _chipSelectPin.State = true;
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

        public async Task SendFireAndForgetAsync(IFireAndForgetOperation operation)
        {
            await WaitForHandshakeAsync();
            var byteCount = operation.Serialize(_writeBuffer);
            _spiBus.Write(_chipSelectPin, _writeBuffer.AsSpan(0, byteCount));
        }

        public async Task<TResponse> SendResponsiveOperationAsync<TResponse>(IResponsiveOperation<TResponse> operation)
            where TResponse : IResponse, new()
        {
            await WaitForHandshakeAsync();
            var byteCount = operation.Serialize(_writeBuffer);
            _spiBus.Write(_chipSelectPin, _writeBuffer.AsSpan(0, byteCount));

            await WaitForHandshakeAsync();
            
            // The first two bytes tells us how many bytes we need to read. We need to keep chip select
            // low in order to read the length without resetting the transaction
            _chipSelectPin.State = false;
            _spiBus.Read(null, _readBuffer.AsSpan(0, 2));
            
            var responseLength = (ushort) (_readBuffer[0] << 8 | _readBuffer[1]);
            _spiBus.Read(null, _readBuffer.AsSpan(0, responseLength));
            _chipSelectPin.State = true;

            var response = new TResponse();
            response.Deserialize(_readBuffer.AsSpan(0, responseLength));

            return response;
        }

        private async Task ResetAsync()
        {
            // Ensure chip select starts high/inactive
            _chipSelectPin.State = true;
            
            // Reset the display
            _resetPin.State = false;
            await Task.Delay(1000);
            _resetPin.State = true;
            
            // Wait for the gpu to be ready
            await WaitForHandshakeAsync();
        }
        
        private async Task WaitForHandshakeAsync()
        {
            var startedAt = DateTime.Now;
            while (!_handshakePin.State)
            {
                if (DateTime.Now - startedAt > TimeSpan.FromSeconds(5))
                {
                    throw new TimeoutException("Timeout waiting for SPI handshake");
                }
                
                await Task.Delay(1);
            }
        }
    }
}