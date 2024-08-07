﻿using Microgpu.Common;
using Microgpu.Common.Operations;

namespace Microgpu.Sample.Common.Samples;

public class Octahedron
{
    private const float RotationSpeed = 150;
    private readonly Camera _camera = new();

    private readonly Gpu _gpu;
    private readonly Vector3 _light = new(1, 0, 3);

    private readonly Triangle[] _octahedron =
    {
        new((1, 0, 0), (0, 1, 0), (0, 0, 1)),
        new((1, 0, 0), (0, 0, -1), (0, 1, 0)),
        new((1, 0, 0), (0, 0, 1), (0, -1, 0)),
        new((1, 0, 0), (0, -1, 0), (0, 0, -1)),
        new((-1, 0, 0), (0, 0, 1), (0, 1, 0)),
        new((-1, 0, 0), (0, 1, 0), (0, 0, -1)),
        new((-1, 0, 0), (0, -1, 0), (0, 0, 1)),
        new((-1, 0, 0), (0, 0, -1), (0, -1, 0))
    };
    
    public Vector3 RotationDegreesPerSecond { get; set; }

    private Vector3 _rotation = new(0, 0, 0);

    public Octahedron(Gpu gpu)
    {
        _gpu = gpu;
    }

    public void RunNextFrame(TimeSpan timeSinceLastFrame)
    {
        _rotation = new Vector3(
            _rotation.X + RotationDegreesPerSecond.X * (float)timeSinceLastFrame.TotalSeconds,
            _rotation.Y + -RotationDegreesPerSecond.Y * (float)timeSinceLastFrame.TotalSeconds,
            _rotation.Z + RotationDegreesPerSecond.Z * (float)timeSinceLastFrame.TotalSeconds);

        var lightUnitVector = _light.Unit;
        var rotatedOctahedron = _octahedron
            .Select(x =>
                new Triangle(x.V1.RotateOnZ(_rotation.Z), x.V2.RotateOnZ(_rotation.Z), x.V3.RotateOnZ(_rotation.Z)))
            .Select(x =>
                new Triangle(x.V1.RotateOnY(_rotation.Y), x.V2.RotateOnY(_rotation.Y), x.V3.RotateOnY(_rotation.Y)))
            .Select(x =>
                new Triangle(x.V1.RotateOnX(_rotation.X), x.V2.RotateOnX(_rotation.X), x.V3.RotateOnX(_rotation.X)))
            .ToArray();

        foreach (var triangle in rotatedOctahedron)
        {
            var projectedTriangle = new Triangle(ProjectTo2d(triangle.V1, _camera),
                ProjectTo2d(triangle.V2, _camera),
                ProjectTo2d(triangle.V3,_camera));

            var normal = triangle.Normal.Unit;

            var alignment = lightUnitVector.Dot(normal);

            if (normal.Z > 0)
            {
                if (alignment < 0) alignment = 0;

                var colorValue = (byte)(alignment * 255);
                var color = ColorRgb565.FromRgb888(colorValue, colorValue, colorValue);

                // draw
                var (x0, y0) = ToScreen(projectedTriangle.V1);
                var (x1, y1) = ToScreen(projectedTriangle.V2);
                var (x2, y2) = ToScreen(projectedTriangle.V3);

                _gpu.EnqueueFireAndForgetAsync(new DrawTriangleOperation<ColorRgb565>
                {
                    TextureId = 0,
                    X0 = x0,
                    Y0 = y0,
                    X1 = x1,
                    Y1 = y1,
                    X2 = x2,
                    Y2 = y2,
                    Color = color
                });
            }
        }
    }

    private (ushort, ushort) ToScreen(Vector3 vector)
    {
        var x = (ushort)(vector.X * 100 + _gpu.FrameBufferResolution!.Value.X / 2);
        var y = (ushort)(vector.Y * 100 + _gpu.FrameBufferResolution!.Value.Y / 2);
        return (x, y);
    }

    private Vector3 ProjectTo2d(Vector3 vector, Camera camera)
    {
        var x = vector.Dot(camera.Right) / camera.Right.Length;
        var y = vector.Dot(camera.Up) / camera.Up.Length;
        return (x, y, 0);
    }
    

    public readonly struct Vector3
    {
        public readonly float X, Y, Z;

        public Vector3(float x, float y, float z)
        {
            (X, Y, Z) = (x, y, z);
        }

        public static implicit operator Vector3((float x, float y, float z) v)
        {
            return new Vector3(v.x, v.y, v.z);
        }

        public static Vector3 operator -(Vector3 first, Vector3 second)
        {
            return new Vector3(first.X - second.X, first.Y - second.Y, first.Z - second.Z);
        }

        public static Vector3 operator +(Vector3 first, Vector3 second)
        {
            return new Vector3(first.X + second.X, first.Y + second.Y, first.Z + second.Z);
        }

        public static Vector3 operator *(Vector3 vec, float scalar)
        {
            return new Vector3(vec.X * scalar, vec.Y * scalar, vec.Z * scalar);
        }

        public float Length => (float)Math.Sqrt(X * X + Y * Y + Z * Z);

        public float Dot(Vector3 other)
        {
            return X * other.X + Y * other.Y + Z * other.Z;
        }

        public Vector3 Unit => this * (1 / Length);

        public Vector3 Cross(Vector3 other)
        {
            var x = Y * other.Z - Z * other.Y;
            var y = Z * other.X - X * other.Z;
            var z = X * other.Y - Y * other.X;
            return new Vector3(x, y, z);
        }

        public Vector3 RotateOnZ(float degrees)
        {
            var rotationRadians = degrees * Math.PI / 180f;
            var currentRotation = Math.Atan2(Y, X);
            var length = (float)Math.Sqrt(X * X + Y * Y);
            var newRotation = currentRotation + rotationRadians;
            var x = (float)Math.Cos(newRotation) * length;
            var y = (float)Math.Sin(newRotation) * length;
            return (x, y, Z);
        }

        public Vector3 RotateOnY(float degrees)
        {
            var rotationRadians = degrees * Math.PI / 180f;
            var currentRotation = Math.Atan2(Z, X);
            var length = (float)Math.Sqrt(X * X + Z * Z);
            var newRotation = currentRotation + rotationRadians;
            var x = (float)Math.Cos(newRotation) * length;
            var z = (float)Math.Sin(newRotation) * length;
            return (x, Y, z);
        }

        public Vector3 RotateOnX(float degrees)
        {
            var rotationRadians = degrees * Math.PI / 180f;
            var currentRotation = Math.Atan2(Y, Z);
            var length = (float)Math.Sqrt(Z * Z + Y * Y);
            var newRotation = currentRotation + rotationRadians;
            var z = (float)Math.Cos(newRotation) * length;
            var y = (float)Math.Sin(newRotation) * length;
            return (X, y, z);
        }

        public override string ToString()
        {
            return $"{{X={X}, Y={Y}, Z={Z}}}";
        }
    }

    private readonly struct Triangle
    {
        public readonly Vector3 V1, V2, V3;

        public Triangle(Vector3 v1, Vector3 v2, Vector3 v3)
        {
            (V1, V2, V3) = (v1, v2, v3);
        }

        public Vector3 Normal => (V2 - V1).Cross(V3 - V1);
    }

    private class Camera
    {
        public readonly Vector3 Right = new(1, 0, 0);
        public readonly Vector3 Up = new(0, 1, 0);
    }
}