#include <stdlib.h>
#include "triangle.h"

typedef struct {
    uint16_t x;
    uint16_t y;
} Point;
typedef struct {
    Point p0, p1, p2;
} Triangle;
typedef struct {
    Point p0, p1;
    int32_t deltaX, deltaY;
    float slope;
} PointPair;

Triangle get_sorted_points(Mgpu_DrawTriangleOperation *operation) {
    Point p1 = {.x = operation->x0, .y = operation->y0};
    Point p2 = {.x = operation->x1, .y = operation->y1};
    Point p3 = {.x = operation->x2, .y = operation->y2};
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

void make_point_pair(PointPair *pair, Point p0, Point p1) {
    pair->p0 = p0;
    pair->p1 = p1;
    pair->deltaX = p1.x - p0.x;
    pair->deltaY = p1.y - p0.y;
    pair->slope = (float) pair->deltaX / (float) pair->deltaY;
}

void draw_triangle(Point top, Point mid, Point bottom, Mgpu_FrameBuffer *frameBuffer, Mgpu_Color color) {
    // Iterate through the triangle from top to bottom one y value at a time.
    // TODO: Swap out for bresenham at some point.

    PointPair topMid, topBottom, midBottom;
    make_point_pair(&topMid, top, mid);
    make_point_pair(&topBottom, top, bottom);
    make_point_pair(&midBottom, mid, bottom);

    PointPair shortPair = topMid;
    PointPair longPair = topBottom;
    float shortX = top.x;
    float longX = top.x;

    for (uint16_t y = top.y; y <= bottom.y; y++) {
        if (y >= frameBuffer->height) {
            break;
        }

        if (y == mid.y) {
            // We reached the mid-point, so swap to the next pair
            shortPair = midBottom;
            shortX = shortPair.p0.x;
        }

        // Draw the row
        int16_t startCol = min(shortX, longX);
        if (startCol < frameBuffer->width) {
            uint16_t diff = longX > shortX ? (int32_t) (longX - shortX) : (int32_t) (shortX - longX);
            int16_t endCol = min(startCol + diff, frameBuffer->width - 1);
            diff = endCol - startCol;

            Mgpu_Color *pixel = frameBuffer->pixels + (y * frameBuffer->width) + startCol;
            for (int x = 0; x <= diff; x++) {
                *pixel = color;
                pixel++;
            }
        }

        // Adjust the x positions
        shortX += shortPair.slope;
        longX += longPair.slope;
    }
}

void mgpu_draw_triangle(Mgpu_DrawTriangleOperation *operation, Mgpu_FrameBuffer *frameBuffer) {
    Triangle triangle = get_sorted_points(operation);
    draw_triangle(triangle.p0, triangle.p1, triangle.p2, frameBuffer, operation->color);
}
