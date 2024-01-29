using System;
using Microgpu.Common.Responses;

namespace Microgpu.Common.Operations;

/// <summary>
///     An operation that can be requested from the Microgpu.
/// </summary>
public interface IOperation
{
    /// <summary>
    ///     Serializes the current operation to the passed in list of bytes.
    ///     The byte list will be cleared during serialization. Serialization should
    ///     be done in network byte order.
    /// </summary>
    /// <param name="bytes">Set of bytes we can serialize this data into</param>
    /// <returns>The number of bytes written into the byte array</returns>
    int Serialize(Span<byte> bytes);
   
    /// <summary>
    ///    Returns the number of bytes required to serialize this operation.
    /// </summary>
    /// <returns></returns>
    int GetSize();
}

/// <summary>
///     An operation that expects a response from the gpu.
/// </summary>
public interface IResponsiveOperation<TResponse> : IOperation where TResponse : IResponse
{
}

/// <summary>
///     An operation that is sent to the gpu without a response coming back
/// </summary>
public interface IFireAndForgetOperation : IOperation
{
}