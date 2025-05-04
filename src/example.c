#include "../vendor/raylib-5.5_macos/include/raylib.h"
#include "moluvi.h"
#include <math.h>
#include <stdint.h>

#define WIDTH 1000
#define HEIGHT 1000
#define CENTER_X (WIDTH / 2.0)
#define CENTER_Y (HEIGHT / 2.0)

#define ANGULAR_SPEED 0.5

void rotate_point(double *x, double *y, double cx, double cy, double theta) {
    double sinT = sin(theta);
    double cosT = cos(theta);

    double xp = *x - cx;
    double yp = *y - cy;
    *x = xp * cosT - yp * sinT + cx;
    *y = xp * sinT + yp * cosT + cy;
}

void CanvasUpdate(Canvas *const canvas, double dt) {
    double angle = ANGULAR_SPEED * dt;

    static double x0 = CENTER_X - 100, y0 = CENTER_Y - 120;
    static double x1 = CENTER_X + 150, y1 = CENTER_Y + 120;
    static double x2 = CENTER_X - 100, y2 = CENTER_Y + 120;
    double cx = (x0 + x1 + x2) / 3;
    double cy = (y0 + y1 + y2) / 3;

    rotate_point(&x0, &y0, cx, cy, angle);
    rotate_point(&x1, &y1, cx, cy, angle);
    rotate_point(&x2, &y2, cx, cy, angle);

    CanvasFill(canvas, COLOR_WHITE);
    CanvasFillQuad(canvas, POINT(50, 300), POINT(400, 40), POINT(500, 700), POINT(700, 200),
                   0xFF34D405); 
    CanvasFillTriangle(canvas, x0, y0, x1, y1, x2, y2, 0xBBFF0000);
}

void PointsExample(Canvas *const canvas, double dt) {
    double angle = ANGULAR_SPEED * dt;
    uint32_t x_interval = WIDTH / 10;
    uint32_t y_interval = HEIGHT / 10;

    CanvasFill(canvas, COLOR_WHITE);
    for (uint32_t x = x_interval / 2; x < canvas->width; x += x_interval) {
        for (uint32_t y = y_interval / 2; y < canvas->height; y += y_interval) {
            double xd = (double)x;
            double yd = (double)y;
            rotate_point(&xd, &yd, WIDTH / 2, HEIGHT / 2, angle);
            CanvasFillCircle(canvas, (int64_t)xd, (int64_t)yd, x_interval / 8, 0xFFD9A377);
        }
    }
}

int main() {
    InitWindow(WIDTH, HEIGHT, "Moluvi Examples");

    Canvas canvas = MakeCanvas(WIDTH, HEIGHT, COLOR_WHITE);
    //CanvasUpdate(&canvas, 0);
    PointsExample(&canvas, 0);

    Image img = (Image){.data = canvas.data,
                        .width = WIDTH,
                        .height = HEIGHT,
                        .mipmaps = 1,
                        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    Texture2D texture = LoadTextureFromImage(img);

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        //CanvasUpdate(&canvas, GetFrameTime());
        PointsExample(&canvas, GetTime());
        UpdateTexture(texture, canvas.data);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTexture(texture, 0, 0, WHITE);
        EndDrawing();
    }

    UnloadTexture(texture);
    CloseWindow();

    return 0;
}
