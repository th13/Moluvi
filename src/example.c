#include "../vendor/raylib-5.5_macos/include/raylib.h"
#include "font_mojangles.h"
#include "moluvi.h"
#include <stddef.h>
#include <stdio.h>

#define WIDTH 1000
#define HEIGHT 1000
#define CENTER_X (WIDTH / 2.0)
#define CENTER_Y (HEIGHT / 2.0)
#define ANGULAR_SPEED 1.0

static MLCamera cam = {
    .dist = 1000,
    .focal_len = 1000,
    .width = WIDTH,
    .height = HEIGHT,
};

void PointsExample(MLCanvas *const canvas, double dt) {
    double angle = ANGULAR_SPEED * dt;
    uint32_t x_interval = WIDTH / 10;
    uint32_t y_interval = HEIGHT / 10;

    MLCanvasFill(canvas, ML_COLOR_BLACK);

    for (uint32_t x = x_interval / 2; x < canvas->width; x += x_interval) {
        for (uint32_t y = y_interval / 2; y < canvas->height; y += y_interval) {
            for (uint32_t z = 0; z < 1000; z += 100) {
                MLPoint3D point = MLPoint3DMake((float)x - CENTER_X,
                                                (float)y - CENTER_Y, (float)z);
                MLPoint3D rot = MLPoint3DRotateY(
                    point, MLPoint3DMake(0, 0, cam.dist), angle);

                double z_depth = rot.z + cam.dist;
                if (z_depth <= 0)
                    continue;

                MLPoint2D proj = MLPoint3DProject(rot, cam);
                float r = MLDistScaleAtZ((float)x_interval / 7., rot.z, cam);

                double x_norm = (double)x / (double)WIDTH;
                double y_norm = (double)y / (double)HEIGHT;
                double z_norm = z / 1000.0;
                MLColor rgba = (MLColor){(uint8_t)lerpd(x_norm, 0, 255),
                                         (uint8_t)lerpd(y_norm, 0, 255),
                                         (uint8_t)lerpd(z_norm, 0, 255), 255};
                MLCanvasFillCircle(canvas, proj.x, proj.y, (uint32_t)r, rgba);
            }
        }
    }
    MLCanvasWriteString(canvas, "CUBE", 30, 30, Mojangles, 3, ML_COLOR_WHITE);
}

#define WORLD_SCALE 60
void TeapotExample(MLCanvas *const canvas, OBJ teapot, double dt) {
    static MLPoint3D center = {0, 0, 0};

    MLCanvasFill(canvas, ML_COLOR_BLACK);
    for (size_t i = 0; i < OBJFaceCount(&teapot); i++) {
        MLVector3Size face = OBJGetFace(&teapot, i);

        MLPoint3D vertices[3] = {
            OBJGetVertex(&teapot, face.x, WORLD_SCALE),
            OBJGetVertex(&teapot, face.y, WORLD_SCALE),
            OBJGetVertex(&teapot, face.z, WORLD_SCALE),
        };

        for (int i = 0; i < 3; i++) {
            vertices[i] =
                MLPoint3DRotateY(vertices[i], center, ANGULAR_SPEED * dt);
        }

        MLPoint2D proj[3] = {
            MLPoint3DProject(vertices[0], cam),
            MLPoint3DProject(vertices[1], cam),
            MLPoint3DProject(vertices[2], cam),
        };

        MLCanvasFillTriangle(canvas, proj[0].x, proj[0].y, proj[1].x, proj[1].y,
                             proj[2].x, proj[2].y, ML_COLOR_RED);
    }
}

// TODO: Z-buffer
// TODO: Lighting
int main() {
    InitWindow(WIDTH, HEIGHT, "Moluvi Examples");

    MLCanvas canvas = MLCanvasMake(WIDTH, HEIGHT, ML_COLOR_WHITE);
    OBJ teapot = OBJLoadFromFile("vendor/cow.obj");
    // PointsExample(&canvas, 0);

    TeapotExample(&canvas, teapot, GetTime());
    Image img = (Image){.data = canvas.data,
                        .width = WIDTH,
                        .height = HEIGHT,
                        .mipmaps = 1,
                        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    Texture2D texture = LoadTextureFromImage(img);

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // PointsExample(&canvas, GetTime());
        TeapotExample(&canvas, teapot, GetTime());
        UpdateTexture(texture, canvas.data);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTexture(texture, 0, 0, WHITE);
        EndDrawing();
    }

    UnloadTexture(texture);
    CloseWindow();

    OBJFree(&teapot);
    MLCanvasDestroy(&canvas);
    return 0;
}
