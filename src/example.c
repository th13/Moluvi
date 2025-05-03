#include "../vendor/raylib-5.5_macos/include/raylib.h"
#include "moluvi.h"
#include <math.h>
#include <stdint.h>

#define WIDTH 1000
#define HEIGHT 1000
#define CENTER_X (WIDTH / 2.)
#define CENTER_Y (HEIGHT / 2.)

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
    CanvasFillTriangle(canvas, x0, y0, x1, y1, x2, y2, COLOR_BLUE);

    // debug
    //char file[32];
    //snprintf(file, 32, "example/debug-%.2f.ppm\n", angle);
    //CanvasRenderPPM(canvas, file);
}

int main() {
    InitWindow(WIDTH, HEIGHT, "Moluvi Examples");

    Canvas canvas = MakeCanvas(WIDTH, HEIGHT, COLOR_WHITE);
    CanvasUpdate(&canvas, 0);

    Image img = (Image){.data = canvas.data,
                        .width = WIDTH,
                        .height = HEIGHT,
                        .mipmaps = 1,
                        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    Texture2D texture = LoadTextureFromImage(img);

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        CanvasUpdate(&canvas, GetFrameTime());
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
