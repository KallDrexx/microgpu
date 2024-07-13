using System;
using System.Threading.Tasks;
using Microgpu.Common.Operations;
using Microgpu.Common.Responses;

namespace Microgpu.Common.Comms;

public interface IGpuCommunication
{
    /// <summary>
    ///     Resets the GPU
    /// </summary>
    ValueTask ResetAsync();

    /// <summary>
    /// Enqueues an operation to be written to the GPU
    /// </summary>
    void EnqueueOutboundOperation(IFireAndForgetOperation operation);

    /// <summary>
    /// Sends any operations that have been queued up to the GPU
    /// </summary>
    ValueTask SendQueuedOutboundOperationsAsync();

    /// <summary>
    /// Immediately sends a single operation to the GPU. Note that this operation will be sent as the only
    /// operation even if other operations have been queued up.
    /// </summary>
    ValueTask SendImmediateOperationAsync(IOperation operation);

    /// <summary>
    /// Reads the next response from the GPU, if one is available
    /// </summary>
    /// <returns></returns>
    ValueTask<TResponse?> ReadNextResponseAsync<TResponse>() where TResponse : class, IResponse, new();
}