using System;

namespace Microgpu.Common.Responses;

public interface IResponse
{
    void Deserialize(ReadOnlySpan<byte> bytes);
}