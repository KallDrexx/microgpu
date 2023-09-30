using Microgpu.Common.Operations;
using Microgpu.Common.Tcp;
using Microgpu.Scratchpad;

Console.WriteLine("Connecting to TCP microgpu server on localhost:9123 ...");
using var client = new MicrogpuTcpClient("localhost", 9123);
await client.ConnectAsync();
Console.WriteLine("Connected");

await client.SendOperationAsync(new InitializeOperation {FrameBufferScale = 1});

Console.WriteLine("Initialization sent");

Console.WriteLine("Starting Octahedron rendering");
await new Octahedron().Run(client);
