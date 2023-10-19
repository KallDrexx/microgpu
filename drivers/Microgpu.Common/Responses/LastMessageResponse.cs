using System;
using System.Text;

namespace Microgpu.Common.Responses;

public class LastMessageResponse : IResponse
{
    public string Message { get; set; } = string.Empty;

    public void Deserialize(ReadOnlySpan<byte> bytes)
    {
        Message = Encoding.ASCII.GetString(bytes);
    }
}