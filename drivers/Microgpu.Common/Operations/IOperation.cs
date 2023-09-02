using System.Collections.Generic;

namespace Microgpu.Common.Operations
{
    /// <summary>
    /// Represents an operation that can be requested from the Microgpu.
    /// </summary>
    public interface IOperation
    {
        /// <summary>
        /// Serializes the current operation to the passed in list of bytes.
        /// The byte list will be cleared during serialization. Serialization should
        /// be done in network byte order.
        /// </summary>
        /// <param name="bytes"></param>
        void Serialize(List<byte> bytes);
    }
}