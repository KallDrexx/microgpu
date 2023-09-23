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
    
    public void Deserialize(ReadOnlySpan<byte> bytes)
    {
        IsInitialized = bytes[0] == 1;
        DisplayWidth = BitConverter.ToUInt16(bytes.Slice(1, 2));
        DisplayHeight = BitConverter.ToUInt16(bytes.Slice(3, 2));
        FrameBufferWidth = BitConverter.ToUInt16(bytes.Slice(5, 2));
        FrameBufferHeight = BitConverter.ToUInt16(bytes.Slice(7, 2));
        ColorMode = (ColorMode)bytes[9];
    }
}