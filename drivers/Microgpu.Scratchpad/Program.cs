using System.Net.Sockets;
using Microgpu.Common;
using Microgpu.Common.Operations;
using Microgpu.Common.Tcp;

using var client = new MicrogpuTcpClient("localhost", 9123);
await client.ConnectAsync();
await client.SendOperationAsync(new InitializeOperation {FrameBufferScale = 1});
await client.SendOperationAsync(new DrawRectangleOperation<ColorRgb565>
{
    StartX = 10,
    StartY = 50,
    Width = 100,
    Height = 200,
    Color = ColorRgb565.FromRgb888(255, 0, 0),
});

await client.SendOperationAsync(new DrawTriangleOperation<ColorRgb565>
{
    X0 = 200,
    Y0 = 10,
    X1 = 50,
    Y1 = 150,
    X2 = 400,
    Y2 = 350,
    Color = ColorRgb565.FromRgb888(0, 255, 0),
});

await client.SendOperationAsync(new PresentFramebufferOperation());

while (true)
{
    await Task.Delay(1000);
}