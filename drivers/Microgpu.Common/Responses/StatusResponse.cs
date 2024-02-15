using System;

namespace Microgpu.Common.Responses;

public class StatusResponse : IResponse
{
    public bool IsInitialized { get; set; }
    public ushort DisplayWidth { get; set; }
    public ushort DisplayHeight { get; set; }
    public ushort FrameBufferWidth { get; set; }
    public ushort FrameBufferHeight { get; set; }
    public ColorMode ColorMode { get; set; }
    public ushort MaxBytes { get; set; }
    public ushort ApiVersionId { get; set; }

    public void Deserialize(ReadOnlySpan<byte> bytes)
    {
        if (bytes[0] != (byte)ResponseType.Status)
        {
            var message = $"Expected type byte of 1 (status response), found {bytes[0]}";
            throw new InvalidOperationException(message);
        }

        IsInitialized = bytes[1] == 1;
        DisplayWidth = (ushort)((bytes[2] << 8) | bytes[3]);
        DisplayHeight = (ushort)((bytes[4] << 8) | bytes[5]);
        FrameBufferWidth = (ushort)((bytes[6] << 8) | bytes[7]);
        FrameBufferHeight = (ushort)((bytes[8] << 8) | bytes[9]);
        ColorMode = (ColorMode)bytes[10];
        MaxBytes = (ushort)((bytes[11] << 8) | bytes[12]);

        // Older microgpu versions do not send the API version
        if (bytes.Length > 13)
        {
            ApiVersionId = (ushort)((bytes[13] << 8) | bytes[14]);
        }
    }
}