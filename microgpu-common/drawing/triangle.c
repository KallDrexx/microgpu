#include <stdlib.h>
#include "triangle.h"

typedef struct {uint16_t x; uint16_t y;} Point;
typedef struct {Point p0, p1, p2;} Triangle;

Triangle get_sorted_points(Mgpu_Op_DrawTriangle *operation) {
    Point p1 = {.x = operation->x0, .y = operation->y0 };
    Point p2 = {.x = operation->x1, .y = operation->y1 };
    Point p3 = {.x = operation->x2, .y = operation->y2 };
    Point temp;

    // Sort the points from top to bottom
    if (p1.y > p2.y) {
        temp = p1;
        p1 = p2;
        p2 = temp;
    }

    if (p3.y < p1.y) {
        temp = p3;
        p3 = p2;
        p2 = p1;
        p1 = temp;
    } else if (p3.y < p2.y) {
        temp = p3;
        p3 = p2;
        p2 = temp;
    }

    Triangle sorted = {.p0 = p1, .p1 = p2, .p2 = p3};
    return sorted;
}

void draw_triangle(Point top, Point left, Point right, Mgpu_FrameBuffer *frameBuffer, Mgpu_Color color) {
    // Iterate through the triangle from top to bottom, filling it in one y position at a time.
    // Keep track of the left and right boundaries from the top. Once we hit the midpoint,
    // swap the top vertex for the midpoint vertex.

    float leftSlope = (float)(left.x - top.x) / (float)(left.y - top.y);
    float rightSlope = (float)(right.x - top.x) / (float)(right.y - top.y);

    float leftX = top.x;
    float rightX = top.x;
    for (uint16_t y = top.y; y <= left.y; y++) {
        uint16_t startX = max(leftX, 0);
        uint16_t endX = max(rightX, frameBuffer->width);

        if (startX <= frameBuffer->width && endX >= 0) {
            size_t diff = endX - startX;
            Mgpu_Color *pixel = frameBuffer->pixels + (y * frameBuffer->width + startX);
            for (size_t x = 0; x < diff; x++) {
                *pixel = color;
                pixel++;
            }
        }

        leftX += leftSlope;
        rightX += rightSlope;
    }
}

void mgpu_draw_triangle(Mgpu_Op_DrawTriangle *operation, Mgpu_FrameBuffer *frameBuffer) {
    Triangle points = get_sorted_points(operation);

    Point top = points.p0;
    Point left, right;
    if (points.p1.x < points.p2.x) {
        left = points.p1;
        right = points.p2;
        right.y = points.p1.y;
    }
}
