using System;
using System.Net.Sockets;
using System.Threading.Tasks;
using Meadow;
using Meadow.Devices;
using Meadow.Hardware;
using Microgpu.Common.Operations;
using Microgpu.Common.Tcp;

namespace Microgpu.Meadow.Scratchpad
{
    public class MeadowApp : App<F7FeatherV2>
    {
        private const string Host = "192.168.0.101";
        private const int Port = 9123;
        private MicrogpuTcpClient _microgpuTcpClient = null!;
        private IWiFiNetworkAdapter _wifi = null!;

        public override async Task Initialize()
        {
            var wifi = Device.NetworkAdapters.Primary<IWiFiNetworkAdapter>();
            if (wifi == null)
            {
                const string message = "No wifi network adapter found!";
                throw new InvalidOperationException(message);
            }

            _wifi = wifi;
            _wifi.NetworkError += (sender, args) => Console.WriteLine($"Network error: {args.ErrorCode}");
            await _wifi.Connect();

            while (!_wifi.IsConnected)
            {
                Console.WriteLine("Waiting for wifi to connect");
                await Task.Delay(1000);
            }

            Console.WriteLine($"Wifi connected with ip {_wifi.IpAddress}");

            var client = new TcpClient();
            await client.ConnectAsync("localhost", 9123);

            Console.WriteLine("TCP client connected");
            var socket = new Socket(SocketType.Stream, ProtocolType.Tcp);
            await socket.ConnectAsync("localhost", 9123);

            Console.WriteLine("Connecting to gpu client");
            _microgpuTcpClient = new MicrogpuTcpClient(Host, Port);
            await _microgpuTcpClient.ConnectAsync();

            Console.WriteLine($"Connected to {Host}:{Port}");
            await _microgpuTcpClient.SendOperationAsync(new InitializeOperation { FrameBufferScale = 1 });
        }
    }
}