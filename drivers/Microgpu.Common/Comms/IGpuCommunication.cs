using System;
using System.Threading.Tasks;

namespace Microgpu.Common.Comms;

public interface IGpuCommunication
{
    /// <summary>
    ///     Resets the GPU
    /// </summary>
    /// <returns></returns>
    Task ResetAsync();

    /// <summary>
    ///     Sends the raw data to the GPU
    /// </summary>
    Task SendDataAsync(Memory<byte> data);

    /// <summary>
    ///     Receives data from the GPU if data is waiting.
    /// </summary>
    /// <returns>The number of bytes received</returns>
    Task<int> ReadDataAsync(Memory<byte> data);
}