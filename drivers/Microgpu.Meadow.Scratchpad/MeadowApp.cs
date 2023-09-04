using System;
using System.Threading.Tasks;
using Meadow;
using Meadow.Devices;
using Meadow.Hardware;

namespace Microgpu.Meadow.Scratchpad
{
    public class MeadowApp : App<F7FeatherV2>
    {
        private IWiFiNetworkAdapter _wifi = null!;

        public override async Task Initialize()
        {
            var wifi = _wifi = Device.NetworkAdapters.Primary<IWiFiNetworkAdapter>()!;
            if (wifi == null)
            {
                const string message = "No wifi network adapter found!";
                throw new InvalidOperationException(message);
            }

            _wifi = wifi;
            while (!_wifi.IsConnected)
            {
                Console.WriteLine("Waiting for wifi to connect");
                await Task.Delay(1000);
            }
            
            Console.WriteLine("Wifi connected");
        }
    }
}