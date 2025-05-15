#include "../vendor/raylib-5.5_macos/include/raylib.h"
#include "font_mojangles.h"
#include "moluvi.h"
#include <math.h>

#define WIDTH 1000
#define HEIGHT 1000
#define CENTER_X (WIDTH / 2.0)
#define CENTER_Y (HEIGHT / 2.0)

#define ANGULAR_SPEED 0.5

void Point3DRotate(double *x, double *y, double *z, double cx, double cy,
                   double cz, double theta) {
    double sinT = sin(theta);
    double cosT = cos(theta);

    double xp = *x - cx;
    double yp = *y - cy;
    double zp = *z - cz;
    double x_z = xp * cosT - zp * sinT;
    double y_z = yp;
    double z_z = xp * sinT + zp * cosT;

    *x = x_z + cx;
    *y = y_z + cy;
    *z = z_z + cz;
}

void MLCanvasUpdate(MLCanvas *const canvas, double dt) {
    double angle = ANGULAR_SPEED * dt;

    static double x0 = CENTER_X - 100, y0 = CENTER_Y - 120;
    static double x1 = CENTER_X + 150, y1 = CENTER_Y + 120;
    static double x2 = CENTER_X - 100, y2 = CENTER_Y + 120;
    double cx = (x0 + x1 + x2) / 3;
    double cy = (y0 + y1 + y2) / 3;

    // Point3DRotate(&x0, &y0, cx, cy, angle);
    // Point3DRotate(&x1, &y1, cx, cy, angle);
    // Point3DRotate(&x2, &y2, cx, cy, angle);

    MLCanvasFill(canvas, ML_COLOR_WHITE);
    MLCanvasFillQuad(canvas, MLPoint2DMake(50, 300), MLPoint2DMake(400, 40),
                     MLPoint2DMake(500, 700), MLPoint2DMake(700, 200),
                     MLColorFromHex(0xFF34D405));
    MLCanvasFillTriangle(canvas, x0, y0, x1, y1, x2, y2,
                         MLColorFromHex(0xBBFF0000));
}

#define FOCAL_LEN 500.0
#define CAM_DIST 500.0

double lerp(double t, double a, double b) { return a + t * (b - a); }

void PointsExample(MLCanvas *const canvas, double dt) {
    double angle = ANGULAR_SPEED * dt;
    uint32_t x_interval = WIDTH / 10;
    uint32_t y_interval = HEIGHT / 10;

    MLCanvasFill(canvas, ML_COLOR_BLACK);

    for (uint32_t x = x_interval / 2; x < canvas->width; x += x_interval) {
        for (uint32_t y = y_interval / 2; y < canvas->height; y += y_interval) {
            for (uint32_t z = 0; z < 1000; z += 100) {
                MLColor color = z % 20 == 0 ? ML_COLOR_RED : ML_COLOR_BLUE;
                double zd = (double)z;
                double xd = (double)x - WIDTH / 2;
                double yd = (double)y - HEIGHT / 2;
                Point3DRotate(&xd, &yd, &zd, 0, 0, 500, angle);

                double z_depth = zd + CAM_DIST;
                if (z_depth <= 0)
                    continue;

                double x_p = (xd * FOCAL_LEN) / z_depth + WIDTH / 2;
                double y_p = (yd * FOCAL_LEN) / z_depth + HEIGHT / 2;
                double r = (((double)x_interval / 8) * FOCAL_LEN) / z_depth;

                double x_norm = (double)x / (double)WIDTH;
                double y_norm = (double)y / (double)HEIGHT;
                double z_norm = z / 1000.0;
                MLColor rgba = (MLColor){(uint8_t)lerp(x_norm, 0, 255),
                                         (uint8_t)lerp(y_norm, 0, 255),
                                         (uint8_t)lerp(z_norm, 0, 255), 255};
                MLCanvasFillCircle(canvas, (int64_t)x_p, (int64_t)y_p,
                                   (uint32_t)r, rgba);
            }
        }
    }
    MLCanvasWriteString(canvas, "CUBE", 30, 30, Mojangles, 3, ML_COLOR_WHITE);
}

int main() {
    InitWindow(WIDTH, HEIGHT, "Moluvi Examples");

    MLCanvas canvas = MLCanvasMake(WIDTH, HEIGHT, ML_COLOR_WHITE);
    PointsExample(&canvas, 0);

    Image img = (Image){.data = canvas.data,
                        .width = WIDTH,
                        .height = HEIGHT,
                        .mipmaps = 1,
                        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    Texture2D texture = LoadTextureFromImage(img);

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
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
