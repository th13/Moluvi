#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#define NOB_IMPLEMENTATION
#include "../nob.h"

#include "moluvi.h"

#define EXAMPLE_DIR "example/"
#define SCALE 10
#define WIDTH 64 * SCALE
#define HEIGHT 48 * SCALE

#define COLOR_DIFF COLOR_RED

typedef enum {
    ANSI_RESET = 0,
    ANSI_BOLD = 1,
    ANSI_BLACK = 30,
    ANSI_RED,
    ANSI_GREEN,
    ANSI_YELLOW,
    ANSI_BLUE,
    ANSI_MAGENTA,
    ANSI_CYAN,
    ANSI_WHITE,
    ANSI_BRIGHT_BLACK = 90,
    ANSI_BRIGHT_RED,
    ANSI_BRIGHT_GREEN,
    ANSI_BRIGHT_YELLOW,
    ANSI_BRIGHT_BLUE,
    ANSI_BRIGHT_MAGENTA,
    ANSI_BRIGHT_CYAN,
    ANSI_BRIGHT_WHITE,
} ANSIEscape;

void ANSIEscapeStdout(int escape) { printf("\033[%dm", escape); }

bool CanvasDiff(const Canvas *const canvas_A, const Canvas *const canvas_B,
                Canvas *canvas_diff) {
    uint32_t diff_width = MAX(canvas_A->width, canvas_B->width);
    uint32_t diff_height = MAX(canvas_A->height, canvas_B->height);
    *canvas_diff = MakeCanvas(diff_width, diff_height, 0);

    bool diff = false;
    for (uint32_t x = 0; x < diff_width; x++) {
        for (uint32_t y = 0; y < diff_height; y++) {
            uint32_t color_A = CanvasGetPixel(canvas_A, x, y);
            uint32_t color_B = CanvasGetPixel(canvas_B, x, y);
            if (color_A != color_B) {
                diff = true;
                CanvasSetPixel(canvas_diff, x, y, COLOR_DIFF);
            } else {
                uint32_t gray = ColorToGrayscale(color_A);
                CanvasSetPixel(canvas_diff, x, y, gray);
            }
        }
    }

    return diff;
}

void TestCanvas(const Canvas *const canvas, const char *ref_file) {
    Canvas canvas_diff = {0};
    Canvas ref_canvas = CanvasLoadPPM(ref_file);
    if (CanvasDiff(canvas, &ref_canvas, &canvas_diff)) {
        char failed_name[64] = EXAMPLE_DIR "FAILED_";
        char ref[64];
        strcpy(ref, ref_file);
        char *token = strtok(ref, "/");
        token = strtok(NULL, "/");
        strcat(failed_name, token);
        CanvasRenderPPM(&canvas_diff, failed_name);
        ANSIEscapeStdout(ANSI_RED);
        printf("❌ TEST %s FAILED! Diff rendered to %s\n", ref_file,
               failed_name);
        ANSIEscapeStdout(ANSI_RESET);
    } else {
        ANSIEscapeStdout(ANSI_GREEN);
        printf("✅ TEST %s SUCCEEDED!\n", ref_file);
        ANSIEscapeStdout(ANSI_RESET);
    }

    CanvasFreeData(&ref_canvas);
    CanvasFreeData(&canvas_diff);
}

typedef enum Subcommand {
    CMD_TEST,
    CMD_REGISTER,
} Subcommand;

typedef void (*ExampleFn)(Canvas *const);

void TestCase(ExampleFn fn, const char *ref_file, Subcommand cmd) {
    printf("Running test case %s\n", ref_file);

    // Set printf to gray for output collected during test
    ANSIEscapeStdout(ANSI_BRIGHT_BLACK);

    Canvas canvas = MakeCanvas(WIDTH, HEIGHT, COLOR_WHITE);
    fn(&canvas);

    if (cmd == CMD_TEST) {
        TestCanvas(&canvas, ref_file);
    } else if (cmd == CMD_REGISTER) {
        // Registers the test case as the new reference data.
        CanvasRenderPPM(&canvas, ref_file);
    }

    CanvasFreeData(&canvas);

    printf("\n");
}

void ShapesExample(Canvas *const canvas) {
    CanvasFillRect(canvas, 4, 4, WIDTH - 8, HEIGHT - 8, 0xFFD9A403);
    CanvasFillCircle(canvas, 0, 0, HEIGHT / 5, 0xBBC35DFA);
}

void LinesExample(Canvas *const canvas) {
    CanvasDrawLine(canvas, 0, 0, WIDTH - 1, HEIGHT - 1, COLOR_BLACK, 1);
    CanvasDrawLine(canvas, WIDTH - 1, 0, 0, HEIGHT - 1, COLOR_BLACK, 1);
    CanvasDrawLine(canvas, WIDTH / 2, 0, WIDTH / 2, HEIGHT - 1, COLOR_BLACK, 1);
    CanvasDrawLine(canvas, 0, HEIGHT / 2, WIDTH - 1, HEIGHT / 2, COLOR_BLACK,
                   1);
    CanvasDrawLine(canvas, 0, 0, WIDTH - 1, HEIGHT / 2, COLOR_BLACK, 1);
    CanvasDrawLine(canvas, WIDTH - 1, 0, 0, HEIGHT / 2, COLOR_BLACK, 1);
    CanvasDrawLine(canvas, 0, HEIGHT - 1, WIDTH - 1, HEIGHT / 2, COLOR_BLACK,
                   1);
    CanvasDrawLine(canvas, WIDTH - 1, HEIGHT - 1, 0, HEIGHT / 2, COLOR_BLACK,
                   1);
}

void ThiccLinesExample(Canvas *const canvas) {
    uint32_t graph_origin_x = WIDTH / 2 - WIDTH / 4;
    // Some lines
    CanvasDrawLine(canvas, graph_origin_x, HEIGHT - 80,
                   HEIGHT - 160 + graph_origin_x - 100, 180, COLOR_BLUE, 1);
    CanvasDrawLine(canvas, graph_origin_x, HEIGHT - 80,
                   HEIGHT - 160 + graph_origin_x - 200, 160, COLOR_RED, 1);
    CanvasDrawLine(canvas, graph_origin_x, HEIGHT - 80,
                   HEIGHT - 160 + graph_origin_x - 40, HEIGHT - 80 - 20,
                   0xFF00DC00, 1);
    // Axes (draw last for Z reasons)
    CanvasDrawLine(canvas, graph_origin_x, 80, graph_origin_x, HEIGHT - 80,
                   COLOR_BLACK, 3);
    CanvasDrawLine(canvas, graph_origin_x, HEIGHT - 80,
                   HEIGHT - 160 + graph_origin_x, HEIGHT - 80, COLOR_BLACK, 3);

    // Labels
    const char *title = "Fruit Consumption vs All-cause Mortality";
    int title_sz = strlen(title);
    CanvasWriteString(canvas, title,
                      WIDTH / 2 - (title_sz * Mojangles.glyph_width) / 2,
                      HEIGHT - 60, Mojangles, 1);
}

void TextExample(Canvas *const canvas) {
    CanvasWriteString(canvas, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 20, HEIGHT / 2,
                      Mojangles, 2);
    CanvasWriteString(canvas, "abcdefghijklmnopqrstuvwxyz", 20, HEIGHT / 2 + 20,
                      Mojangles, 1);
}

void TriangleExample(Canvas *const canvas) {
    CanvasFillTriangle(canvas, 0, 0, WIDTH / 2, 10, WIDTH / 2 - 10, HEIGHT / 2,
                       COLOR_BLACK);
    CanvasFillTriangle(canvas, 30, 40, 200, 70, 4, 369, 0xBA2308FF);
    CanvasFillTriangle(canvas, WIDTH / 2 - 90, HEIGHT / 2 - 20, 550,
                       HEIGHT / 2 - 90, 550, HEIGHT / 2 + 100, 0xC3B8D908);
    CanvasWriteString(canvas, "THESE ARE TRIANGLES", 10, HEIGHT - 20, Mojangles,
                      2);
}

int main(int argc, char **argv) {
    nob_mkdir_if_not_exists(EXAMPLE_DIR);

    // Parse command. Default mode is CMD_TEST
    nob_shift(argv, argc);
    Subcommand cmd = CMD_TEST;
    if (argc) {
        const char *command = nob_shift(argv, argc);
        if (strcmp(command, "register") == 0) {
            cmd = CMD_REGISTER;
        } else {
            printf("Illegal subcommand: %s\n", command);
        }
    }

    TestCase(&ShapesExample, "example/shapes.ppm", cmd);
    TestCase(&LinesExample, "example/lines.ppm", cmd);
    TestCase(&ThiccLinesExample, "example/thicc.ppm", cmd);
    TestCase(&TextExample, "example/text.ppm", cmd);
    TestCase(&TriangleExample, "example/tri.ppm", cmd);
    return 0;
}
