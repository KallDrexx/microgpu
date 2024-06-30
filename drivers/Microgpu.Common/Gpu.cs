using System;
using System.Numerics;
using System.Threading.Tasks;
using Microgpu.Common.Comms;
using Microgpu.Common.Operations;
using Microgpu.Common.Responses;

namespace Microgpu.Common;

public class Gpu
{
    public const ushort ValidApiVersionId = 2;
    
    private readonly IGpuCommunication _communication;
    private byte[] _readBuffer = new byte[1024];
    private byte[] _writeBuffer = new byte[1024];

    private Gpu(IGpuCommunication communication)
    {
        _communication = communication;
    }

    /// <summary>
    ///     Indicates whether the GPU is initialized. If the GPU is not initialized
    ///     most operations sent to it will do nothing.
    /// </summary>
    public bool IsInitialized { get; private set; }

    /// <summary>
    ///     What color mode the GPU is expecting color data in.
    /// </summary>
    public ColorMode ColorMode { get; private set; }

    /// <summary>
    ///     The resolution of the display in pixels
    /// </summary>
    public Vector2 DisplayResolution { get; private set; }

    /// <summary>
    ///     The resolution of the frame buffer in pixels. This will be null if the GPU is not initialized.
    /// </summary>
    public Vector2? FrameBufferResolution { get; private set; }
    
    /// <summary>
    ///     Creates a new GPU instance and initializes it
    /// </summary>
    /// <param name="communication">The communication protocol to use to talk to the GPU</param>
    public static async Task<Gpu> CreateAsync(IGpuCommunication communication)
    {
        if (communication == null) throw new ArgumentNullException(nameof(communication));

        await communication.ResetAsync();
        var gpu = new Gpu(communication);
        await GetAndApplyStatus(gpu);

        return gpu;
    }

    /// <summary>
    ///     Initializes the GPU if it has not already been done.
    /// </summary>
    public async Task InitializeAsync(byte framebufferScale)
    {
        if (IsInitialized) return;
        
        await _communication.SendImmediateOperationAsync(new InitializeOperation { FrameBufferScale = framebufferScale });
        await GetAndApplyStatus(this);

        if (!IsInitialized)
        {
            const string message = "Attempted to initialize the GPU, but it is not initialized";
            throw new InvalidOperationException(message);
        }
    }

    /// <summary>
    /// Enqueues an operation to be executed at another time
    /// </summary>
    /// <param name="operation"></param>
    /// <exception cref="ArgumentNullException"></exception>
    public void EnqueueFireAndForgetAsync(IFireAndForgetOperation operation)
    {
        operation = operation ?? throw new ArgumentNullException(nameof(operation));
        _communication.EnqueueOutboundOperation(operation);
    }

    /// <summary>
    /// Sends all operations that have been queued on the gpu communication line
    /// </summary>
    public async ValueTask SendQueuedOperationsAsync()
    {
        await _communication.SendQueuedOutboundOperationsAsync();
    }

    /// <summary>
    /// Sends an operation to the GPU and deserializes the response.
    /// </summary>
    public async ValueTask<TResponse?> SendResponsiveOperationAsync<TResponse>(IResponsiveOperation<TResponse> operation)
        where TResponse : class, IResponse, new()
    {
        operation = operation ?? throw new ArgumentNullException(nameof(operation));

        await _communication.SendImmediateOperationAsync(operation);
        return await _communication.ReadNextResponseAsync<TResponse>();
    }

    private static async Task GetAndApplyStatus(Gpu gpu)
    {
        var status = await gpu.SendResponsiveOperationAsync(new GetStatusOperation());
        if (status != null)
        {
            gpu.ApplyStatusResponse(status);
        }
        else
        {
            Console.WriteLine("Warning: Status requested but no status was returned by the GPU");
        }
    }

    private void ApplyStatusResponse(StatusResponse status)
    {
        ColorMode = status.ColorMode;
        IsInitialized = status.IsInitialized;
        DisplayResolution = new Vector2(status.DisplayWidth, status.DisplayHeight);

        if (IsInitialized)
            FrameBufferResolution = new Vector2(status.FrameBufferWidth, status.FrameBufferHeight);
        else
            FrameBufferResolution = null;

        if (status.MaxBytes > 0)
        {
            _writeBuffer = new byte[status.MaxBytes];
            _readBuffer = new byte[status.MaxBytes];
        }
        
        if (status.ApiVersionId != ValidApiVersionId)
        {
            var message = $"The GPU reported an API version is {status.ApiVersionId} but this " +
                          $"library only supports version {ValidApiVersionId}";
            
            throw new InvalidOperationException(message);
        }
    }
}