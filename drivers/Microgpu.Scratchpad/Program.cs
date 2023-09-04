using Microgpu.Common;
using Microgpu.Common.Operations;
using Microgpu.Common.Tcp;
using Microgpu.Scratchpad;

using var client = new MicrogpuTcpClient("localhost", 9123);
await client.ConnectAsync();
await client.SendOperationAsync(new InitializeOperation {FrameBufferScale = 1});
await new Octahedron().Run(client);
