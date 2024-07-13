using System.Numerics;
using Microgpu.Common;
using Microgpu.Common.Operations;

namespace Microgpu.Sample.Common.Samples;

public class BouncingTexture
{
    private const int Speed = 150;
    private readonly Gpu _gpu;

    private readonly TextureManager.Texture _texture;
    private HorizontalDirection _horizontalDirection;
    private VerticalDirection _verticalDirection;
    private Vector2 _position;

    public BouncingTexture(Gpu gpu, TextureManager.Texture texture)
    {
        _texture = texture;
        _gpu = gpu;

        var random = new Random();
        _position = new Vector2(
            (float)random.NextDouble() * gpu.FrameBufferResolution!.Value.X,
            (float)random.NextDouble() * gpu.FrameBufferResolution!.Value.Y);
        
        _horizontalDirection = random.Next(0, 2) == 0 ? HorizontalDirection.Left : HorizontalDirection.Right;
        _verticalDirection = random.Next(0, 2) == 0 ? VerticalDirection.Up : VerticalDirection.Down;
    }

    public void RunNextFrame(TimeSpan frameTime)
    {
        var horizontalMovement = (float)(Speed * frameTime.TotalSeconds);
        var verticalMovement = (float)(Speed * frameTime.TotalSeconds);

        if (_horizontalDirection == HorizontalDirection.Left) horizontalMovement *= -1;

        if (_verticalDirection == VerticalDirection.Up) verticalMovement *= -1;

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
        
        _gpu.EnqueueFireAndForgetAsync(new DrawTextureOperation
        {
            SourceTextureId = _texture.Id,
            TargetTextureId = 0,
            SourceStartX = 0,
            SourceStartY = 0,
            SourceWidth = _texture.Width,
            SourceHeight = _texture.Height,
            TargetStartX = (short)_position.X,
            TargetStartY = (short)_position.Y,
            IgnoreTransparency = false,
        });
    }

    private enum HorizontalDirection
    {
        Left,
        Right
    }

    private enum VerticalDirection
    {
        Up,
        Down
    }
}