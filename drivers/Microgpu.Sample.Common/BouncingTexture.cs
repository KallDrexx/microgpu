using System.Numerics;
using Microgpu.Common;
using Microgpu.Common.Operations;

namespace Microgpu.Sample.Common;

public class BouncingTexture
{
    private enum HorizontalDirection { Left, Right }
    private enum VerticalDirection { Up, Down }

    private const int Speed = 0;

    private readonly byte _textureId;
    private readonly Gpu _gpu;
    private Vector2 _position;
    private HorizontalDirection _horizontalDirection = HorizontalDirection.Right;
    private VerticalDirection _verticalDirection = VerticalDirection.Down;

    public BouncingTexture(Gpu gpu, byte textureId)
    {
        _textureId = textureId;
        _gpu = gpu;

        var random = new Random();
        _position = new Vector2(
            1000,//(float)random.NextDouble() * gpu.FrameBufferResolution!.Value.X,
            0); //(float)random.NextDouble() * gpu.FrameBufferResolution!.Value.Y);
    }
    
    public async Task RunNextFrameAsync(TimeSpan frameTime)
    {
        var horizontalMovement = (float)(Speed * frameTime.TotalSeconds);
        var verticalMovement = (float)(Speed * frameTime.TotalSeconds);

        if (_horizontalDirection == HorizontalDirection.Left)
        {
            horizontalMovement *= -1;
        }

        if (_verticalDirection == VerticalDirection.Up)
        {
            verticalMovement *= -1;
        }

        _position.X += horizontalMovement;
        _position.Y += verticalMovement;

        if (_position.X < 0)
        {
            _position.X = 0;
            _horizontalDirection = HorizontalDirection.Right;
        }
        else if (_position.X > _gpu.FrameBufferResolution!.Value.X)
        {
            _position.X = _gpu.FrameBufferResolution!.Value.X;
            _horizontalDirection = HorizontalDirection.Left;
        }

        if (_position.Y < 0)
        {
            _position.Y = 0;
            _verticalDirection = VerticalDirection.Down;
        }
        else if (_position.Y > _gpu.FrameBufferResolution!.Value.Y)
        {
            _position.Y = _gpu.FrameBufferResolution!.Value.Y;
            _verticalDirection = VerticalDirection.Up;
        }

        await _gpu.SendFireAndForgetAsync(new DrawTextureOperation
        {
            TextureId = _textureId,
            X = (short)_position.X,
            Y = (short)_position.Y,
        });
    }
}