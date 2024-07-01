using System;
using System.Diagnostics;
using System.Threading.Tasks;
using Meadow.Hardware;

namespace Microgpu.Common.Comms;

public class MeadowSpiGpuCommunication : IGpuCommunication
{
    private readonly IDigitalOutputPort _chipSelectPin;
    private readonly IDigitalInputPort _handshakePin;
    private readonly byte[] _readLengthBuffer = new byte[2];
    private readonly IDigitalOutputPort _resetPin;
    private readonly ISpiBus _spiBus;

    public MeadowSpiGpuCommunication(
        ISpiBus spiBus,
        IDigitalInputPort handshakePin,
        IDigitalOutputPort resetPin,
        IDigitalOutputPort chipSelectPin)
    {
        _spiBus = spiBus;
        _handshakePin = handshakePin;
        _resetPin = resetPin;
        _chipSelectPin = chipSelectPin;
    }

    public async Task ResetAsync()
    {
        // Ensure the chip select starts high/inactive
        _chipSelectPin.State = true;

        // Reset the display
        _resetPin.State = false;
        await Task.Delay(1000);
        _resetPin.State = true;

        // Wait for the gpu to signal it's ready
        await WaitForHandshakeAsync();
    }

    public async Task SendDataAsync(Memory<byte> data)
    {
        await WaitForHandshakeAsync();
        _spiBus.Write(_chipSelectPin, data.Span);
    }

    public async Task<int> ReadDataAsync(Memory<byte> data)
    {
        await WaitForHandshakeAsync();

        // The first two bytes tells us how many bytes we need to read. We need to keep chip select
        // low in order to read the length without resetting the transaction
        _chipSelectPin.State = false;
        _spiBus.Read(null, _readLengthBuffer);
       
        var responseLength = (ushort)((_readLengthBuffer[0] << 8) | _readLengthBuffer[1]);
        // If we receive all 1s, that mostly likely CIPO isn't connected, and thus we have no data
        responseLength = responseLength == 0xFFFF ? (ushort)0 : responseLength;

        if (responseLength is not 0)
        {
            try
            {
                _spiBus.Read(null, data.Span[..responseLength]);
            }
            catch
            {
                Console.WriteLine($"Exception occurred.  RepsonseLength == {responseLength}");
                Console.WriteLine($"{_readLengthBuffer[0]:x2} {_readLengthBuffer[1]:x2}");
                throw;
            }
        }
        
        _chipSelectPin.State = true;

        return responseLength;
    }

    private async Task WaitForHandshakeAsync()
    {
        var startedAt = DateTime.Now;
        while (!_handshakePin.State)
        {
            if (DateTime.Now - startedAt > TimeSpan.FromSeconds(5))
                throw new TimeoutException("Timeout waiting for SPI handshake");

            await Task.Yield();
        }
    }
}