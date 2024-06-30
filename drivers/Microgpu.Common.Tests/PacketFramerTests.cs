using Microgpu.Common.Operations;
using Shouldly;

namespace Microgpu.Common.Tests;

public class PacketFramerTests
{
    [Theory]
    [MemberData(nameof(ValidDecodeTestCases))]
    public void Can_Decode_Valid_Packet(byte[] input, byte[] expected)
    {
        var framer = new PacketFramer();
        var result = framer.Decode(input.AsSpan());
        result.DecodedBytes.ToArray().ShouldBeEquivalentTo(expected);
    }
    
    [Theory]
    [MemberData(nameof(InvalidDecodeTestCases))]
    public void Correctly_Fails_To_Decode_Bad_Cases(byte[] input)
    {
        var framer = new PacketFramer();
        var result = framer.Decode(input.AsSpan());
        result.DecodedBytes.ToArray().ShouldBeEmpty();
    }

    [Theory]
    [MemberData(nameof(EncodeTestCases))]
    public void Can_Encode_Operations(IOperation operation, byte[] expected)
    {
        var framer = new PacketFramer();
        var outputBuffer = new byte[255];
        var result = framer.Encode(operation, outputBuffer.AsSpan());
        result.ShouldBe(expected.Length);
        outputBuffer[..result].ToArray().ShouldBeEquivalentTo(expected);
    }

    public static IEnumerable<object[]> ValidDecodeTestCases()
    {
        yield return [new byte[] { 1, 1, 1, 1, 0 }, new byte[] { 0 }];
        yield return [new byte[] { 1, 1, 1, 1, 1, 0 }, new byte[] { 0, 0 }];
        yield return [new byte[] { 0x1, 0x2, 0x11, 0x1, 0x02, 0x11, 0x0 }, new byte[] { 0, 0x11, 0 }];
        yield return [new byte[] { 0x3, 0x11, 0x22, 0x2, 0x33, 0x02, 0x66, 0 }, new byte[] { 0x11, 0x22, 0x00, 0x33 }];
        yield return
            [new byte[] { 0x05, 0x11, 0x22, 0x33, 0x44, 0x02, 0xaa, 0x00 }, new byte[] { 0x11, 0x22, 0x33, 0x44 }];
        
        yield return
            [new byte[] { 0x02, 0x11, 0x01, 0x01, 0x01, 0x02, 0x11, 0x00 }, new byte[] { 0x11, 0x00, 0x00, 0x00 }];

        yield return [new byte[] { 0x05, 0xff, 0x03, 0x01, 0x02, 0x00 }, new byte[] { 0xff, 0x03 }];
    }

    public static IEnumerable<object[]> InvalidDecodeTestCases()
    {
        yield return [new byte[] { 0x1, 0x2, 0x11, 0x1, 0x00, 0x11, 0x0 }]; // Invalid zero in the middle
        yield return [new byte[] { 0x4, 0x1, 0x0 }]; // Initial offset past length
        yield return [new byte[] { 0x2, 0x1, 0x2, 0x0 }]; // second offset past length
        yield return [new byte[] { 0x2, 0x1, 0x2, 0x2, 0x0 }]; // failed checksum
    }

    public static IEnumerable<object[]> EncodeTestCases()
    {
        yield return [new GetStatusOperation(), new byte[] { 0x2, 0x4, 0x2, 0x4, 0x0 }];
    }
}