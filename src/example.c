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

static struct camera cam = {
    .dist = 1000,
    .focal_len = 1000,
    .width = WIDTH,
    .height = HEIGHT,
};

void points_example(canvas_t *const canvas, double dt) {
    double angle = ANGULAR_SPEED * dt;
    uint32_t x_interval = WIDTH / 10;
    uint32_t y_interval = HEIGHT / 10;

    canvas_fill(canvas, COLOR_BLACK);

    point3_t center = {0, 0, cam.dist};
    for (uint32_t x = x_interval / 2; x < canvas->width; x += x_interval) {
        for (uint32_t y = y_interval / 2; y < canvas->height; y += y_interval) {
            for (uint32_t z = 0; z < 1000; z += 100) {
                point3_t point = {(float)x - CENTER_X, (float)y - CENTER_Y,
                                  (float)z};
                point3_rotate(&point, center, angle);

                double z_depth = point.z + cam.dist;
                if (z_depth <= 0)
                    continue;

                point2_t proj = point3_proj(point, cam);
                float r = scale_z((float)x_interval / 7., point.z, cam);

                double x_norm = (double)x / (double)WIDTH;
                double y_norm = (double)y / (double)HEIGHT;
                double z_norm = z / 1000.0;
                struct rgba color =
                    (struct rgba){(uint8_t)lerpd(x_norm, 0, 255),
                                  (uint8_t)lerpd(y_norm, 0, 255),
                                  (uint8_t)lerpd(z_norm, 0, 255), 255};
                canvas_fill_circle(canvas, proj.x, proj.y, (uint32_t)r, color);
            }
        }
    }
    canvas_write_string(canvas, "CUBE", 30, 30, font_mojangles, 3, COLOR_WHITE);
}

#define WORLD_SCALE 60
void obj_example(canvas_t *const canvas, obj_t teapot, double dt) {
    static point3_t center = {0, 0, 0};

    canvas_fill(canvas, COLOR_BLACK);
    canvas_depth_reset(canvas);
    for (size_t i = 0; i < obj_face_count(&teapot); i++) {
        struct vec3z face = obj_get_face(&teapot, i);

        point3_t vertices[3] = {
            obj_get_vertex(&teapot, face.x, WORLD_SCALE),
            obj_get_vertex(&teapot, face.y, WORLD_SCALE),
            obj_get_vertex(&teapot, face.z, WORLD_SCALE),
        };

        for (int i = 0; i < 3; i++) {
            point3_rotate(&vertices[i], center, ANGULAR_SPEED * dt);
        }

        canvas_proj_tri(canvas, vertices, cam);
    }
}

// TODO: Lighting
int main() {
    int ret;
    InitWindow(WIDTH, HEIGHT, "Moluvi Examples");

    canvas_t canvas;
    canvas_init(&canvas, WIDTH, HEIGHT, COLOR_WHITE);
    canvas_use_depth(&canvas);

    obj_t teapot;
    ret = obj_load(&teapot, "vendor/cow.obj");
    if (ret < 0)
        return ret;
    // points_example(&canvas, 0);

    obj_example(&canvas, teapot, GetTime());
    Image img = (Image){.data = canvas.data,
                        .width = WIDTH,
                        .height = HEIGHT,
                        .mipmaps = 1,
                        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    Texture2D texture = LoadTextureFromImage(img);

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // points_example(&canvas, GetTime());
        obj_example(&canvas, teapot, GetTime());
        UpdateTexture(texture, canvas.data);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTexture(texture, 0, 0, WHITE);
        EndDrawing();
    }

    UnloadTexture(texture);
    CloseWindow();

    obj_cleanup(&teapot);
    canvas_cleanup(&canvas);
    return ret;
}
