#define NOB_IMPLEMENTATION
#include "../nob.h"
#include "moluvi.h"

#define EXAMPLE_DIR "example/"
#define SCALE 25
#define WIDTH 64 * SCALE
#define HEIGHT 48 * SCALE

int main() {
    nob_mkdir_if_not_exists(EXAMPLE_DIR);

    Canvas canvas = MakeCanvas(WIDTH, HEIGHT, 0xFF8732FF);
    CanvasFillRect(&canvas, 4, 4, WIDTH - 8, HEIGHT - 8, 0x03A4D9FF);
    CanvasFillCircle(&canvas, 0, 0, HEIGHT / 5, 0xFA5DC3BB);
    CanvasRenderPPM(&canvas, EXAMPLE_DIR "test.ppm");
    CanvasDestroy(&canvas);

    canvas = MakeCanvas(WIDTH, HEIGHT, 0x03A4D9FF);
    CanvasDrawLine(&canvas, 0, 0, WIDTH - 1, HEIGHT - 1, COLOR_BLACK);
    CanvasDrawLine(&canvas, WIDTH - 1, 0, 0, HEIGHT - 1, COLOR_BLACK);
    CanvasDrawLine(&canvas, WIDTH / 2, 0, WIDTH / 2, HEIGHT, COLOR_BLACK);
    CanvasDrawLine(&canvas, 0, HEIGHT / 2, WIDTH - 1, HEIGHT / 2, COLOR_BLACK);
    CanvasDrawLine(&canvas, 0, 0, WIDTH - 1, HEIGHT / 2, COLOR_BLACK);
    CanvasDrawLine(&canvas, WIDTH - 1, 0, 0, HEIGHT / 2, COLOR_BLACK);
    CanvasDrawLine(&canvas, 0, HEIGHT - 1, WIDTH - 1, HEIGHT / 2, COLOR_BLACK);
    CanvasDrawLine(&canvas, WIDTH - 1, HEIGHT - 1, 0, HEIGHT / 2, COLOR_BLACK);
    CanvasRenderPPM(&canvas, EXAMPLE_DIR "lines.ppm");
    CanvasDestroy(&canvas);

    canvas = MakeCanvas(WIDTH, HEIGHT, COLOR_WHITE);
    CanvasWriteString(&canvas, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 20, HEIGHT / 2,
                      Mojangles, 2);
    CanvasWriteString(&canvas, "abcdefghijklmnopqrstuvwxyz", 20,
                      HEIGHT / 2 + 20, Mojangles, 1);
    CanvasRenderPPM(&canvas, EXAMPLE_DIR "text.ppm");
    CanvasDestroy(&canvas);

    canvas = MakeCanvas(WIDTH, HEIGHT, COLOR_WHITE);
    CanvasFillTriangle(&canvas, 0, 0, WIDTH / 2, 10, WIDTH / 2 - 10, HEIGHT / 2,
                       COLOR_BLACK);
    CanvasFillTriangle(&canvas, 30, 40, 200, 70, 4, 369, 0xFF0823BA);
    CanvasFillTriangle(&canvas, WIDTH / 2 - 90, HEIGHT / 2 - 20, 550,
                       HEIGHT / 2 - 90, 550, HEIGHT / 2 + 100, 0x08D9B8C3);
    CanvasWriteString(&canvas, "THESE ARE TRIANGLES", 10, HEIGHT - 20,
                      Mojangles, 2);
    CanvasRenderPPM(&canvas, EXAMPLE_DIR "tri.ppm");
    CanvasDestroy(&canvas);
    return 0;
}
