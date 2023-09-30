using Microgpu.Common;
using Microgpu.Common.Operations;
using Microgpu.Common.Tcp;

namespace Microgpu.Scratchpad;

public class Octahedron
{
    public async Task Run(MicrogpuTcpClient client) {
        var camera = new Camera();
        var octehedron = new[] {
            new Triangle((1, 0, 0), (0, 1, 0), (0, 0, 1)),
            new Triangle((1, 0, 0), (0, 0, -1), (0, 1, 0)),
            new Triangle((1, 0, 0), (0, 0, 1), (0, -1, 0)),
            new Triangle((1, 0, 0), (0, -1, 0), (0, 0, -1)),
            new Triangle((-1, 0, 0), (0, 0, 1), (0, 1, 0)),
            new Triangle((-1, 0, 0), (0, 1, 0), (0, 0, -1)),
            new Triangle((-1, 0, 0), (0, -1, 0), (0, 0, 1)),
            new Triangle((-1, 0, 0), (0, 0, -1), (0, -1, 0)),
        };

        var zRotation = 0;
        var yRotation = 0;
        var xRotation = 0;
        var light = new Vector3(1, 0, 3);
        var lightUnitVector = light.Unit;

        var gpuBatch = new BatchOperation();
        while(true)
        {
            var rotatedOctehedron = octehedron
                .Select(x => new Triangle(x.V1.RotateOnZ(zRotation), x.V2.RotateOnZ(zRotation), x.V3.RotateOnZ(zRotation)))
                .Select(x => new Triangle(x.V1.RotateOnY(yRotation), x.V2.RotateOnY(yRotation), x.V3.RotateOnY(yRotation)))
                .Select(x => new Triangle(x.V1.RotateOnX(xRotation), x.V2.RotateOnX(xRotation), x.V3.RotateOnX(xRotation)))
                .ToArray();

            foreach (var triangle in rotatedOctehedron) {
                var projectedTriangle = new Triangle(triangle.V1.ProjectTo2d(camera), 
                    triangle.V2.ProjectTo2d(camera), 
                    triangle.V3.ProjectTo2d(camera));

                var normal = triangle.Normal.Unit;

                var alignment = lightUnitVector.Dot(normal);

                if (normal.Z > 0) {
                    if (alignment < 0) {
                        alignment = 0;
                    }

                    var colorValue = (byte)(alignment * 255);
                    var color = ColorRgb565.FromRgb888(colorValue, colorValue, colorValue);

                    // draw
                    var (x0, y0) = ToScreen(projectedTriangle.V1);
                    var (x1, y1) = ToScreen(projectedTriangle.V2);
                    var (x2, y2) = ToScreen(projectedTriangle.V3);
                    
                    gpuBatch.AddOperation(new DrawTriangleOperation<ColorRgb565>
                    {
                        X0 = x0,
                        Y0 = y0,
                        X1 = x1,
                        Y1 = y1,
                        X2 = x2,
                        Y2 = y2,
                        Color = color,
                    });
                }
            }

            yRotation += 5;
            xRotation += 5;

            gpuBatch.AddOperation(new PresentFramebufferOperation());
            await client.SendOperationAsync(gpuBatch);
            await Task.Delay(16);
            // return;
        }   
    }

    private (ushort, ushort) ToScreen(Vector3 vector)
    {
        var x = (ushort) (vector.X * 100 + 320);
        var y = (ushort) (vector.Y * 100 + 240);
        return (x, y);
    }

    private readonly struct Vector3 {
        public readonly float X, Y, Z;

        public Vector3(float x, float y, float z) => (X, Y, Z) = (x, y, z);
        
        public static implicit operator Vector3((float x, float y, float z) v) => new Vector3(v.x, v.y, v.z);
        public static Vector3 operator-(Vector3 first, Vector3 second) => new Vector3(first.X - second.X, first.Y - second.Y, first.Z - second.Z);
        public static Vector3 operator+(Vector3 first, Vector3 second) => new Vector3(first.X + second.X, first.Y + second.Y, first.Z + second.Z);
        public static Vector3 operator*(Vector3 vec, float scalar) => new Vector3(vec.X * scalar, vec.Y * scalar, vec.Z * scalar);

        public (float x, float y) ToGraphPoint() => (X, Y);
        public float Length => (float) Math.Sqrt(X * X + Y * Y + Z * Z);
        public float Dot(Vector3 other) => (X * other.X) + (Y * other.Y) + (Z * other.Z);
        public Vector3 Unit => this * (1 / this.Length);

        public Vector3 Cross(Vector3 other) {
            var x = Y * other.Z - Z * other.Y;
            var y = Z * other.X - X * other.Z;
            var z = X * other.Y - Y * other.X;
            return (new Vector3(x, y, z));
        }

        public Vector3 ProjectTo2d(Camera camera) {
            var x = this.Dot(camera.Right) / camera.Right.Length;
            var y = this.Dot(camera.Up) / camera.Up.Length;
            return (x, y, 0);
        }

        public Vector3 RotateOnZ(int degrees) {
            var rotationRadians = degrees * Math.PI / 180f;
            var currentRotation = Math.Atan2(Y, X);
            var length = (float) Math.Sqrt(X*X + Y*Y);
            var newRotation = currentRotation + rotationRadians;
            var x = (float) Math.Cos(newRotation) * length;
            var y = (float) Math.Sin(newRotation) * length;
            return (x, y, Z);
        }

        public Vector3 RotateOnY(int degrees) {
            var rotationRadians = degrees * Math.PI / 180f;
            var currentRotation = Math.Atan2(Z, X);
            var length = (float) Math.Sqrt(X*X + Z*Z);
            var newRotation = currentRotation + rotationRadians;
            var x = (float) Math.Cos(newRotation) * length;
            var z = (float) Math.Sin(newRotation) * length;
            return (x, Y, z);
        }

        public Vector3 RotateOnX(int degrees) {
            var rotationRadians = degrees * Math.PI / 180f;
            var currentRotation = Math.Atan2(Y, Z);
            var length = (float) Math.Sqrt(Z*Z + Y*Y);
            var newRotation = currentRotation + rotationRadians;
            var z = (float) Math.Cos(newRotation) * length;
            var y = (float) Math.Sin(newRotation) * length;
            return (X, y, z);
        }

        public override string ToString() => $"{{X={X}, Y={Y}, Z={Z}}}";
    }

    private readonly struct Triangle {
        public readonly Vector3 V1, V2, V3;

        public Triangle(Vector3 v1, Vector3 v2, Vector3 v3) => (V1, V2, V3) = (v1, v2, v3);
        public Vector3 Normal => (V2 - V1).Cross(V3 - V1);
    }

    private class Camera {
        public Vector3 Up, Right;

        public Camera() {
            Up = (0, 1, 0);
            Right = (1, 0, 0);
        }
    }
}