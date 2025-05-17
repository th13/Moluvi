#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define NOB_IMPLEMENTATION
#include "../nob.h"

#include "font_mojangles.h"
#include "moluvi.h"

#define TEST_DIR "test/"
#define SCALE 10
#define WIDTH 64 * SCALE
#define HEIGHT 48 * SCALE

#define COLOR_DIFF ML_COLOR_RED

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

typedef enum Subcommand {
    CMD_RUN,
    CMD_REGISTER,
} Subcommand;

typedef enum DiffMode {
    DIFF_MODE_GRAYSCALE, // Render the output in grayscale, highlighting diff in
                         // red
    DIFF_MODE_OURS,      // Render our test output, with no diff compared to the
                         // original
    DIFF_MODE_BLEND_DIFF // Render the output using difference blending (i.e.
                         // |fg - bg|)
} DiffMode;

typedef void (*ExampleFn)(MLCanvas *const);

bool MLCanvasDiff(const MLCanvas *const canvas_A,
                  const MLCanvas *const canvas_B, MLCanvas *canvas_diff,
                  DiffMode diff_mode) {
    uint32_t diff_width = MAX(canvas_A->width, canvas_B->width);
    uint32_t diff_height = MAX(canvas_A->height, canvas_B->height);
    *canvas_diff = MLCanvasMake(diff_width, diff_height, ML_COLOR_BLACK);

    bool diff = false;
    for (uint32_t x = 0; x < diff_width; x++) {
        for (uint32_t y = 0; y < diff_height; y++) {
            MLColor color_A = MLCanvasGetPixel(canvas_A, x, y);
            MLColor color_B = MLCanvasGetPixel(canvas_B, x, y);
            bool color_eql = MLColorEqual(color_A, color_B);
            if (!color_eql)
                diff = true;

            if (diff_mode == DIFF_MODE_GRAYSCALE) {
                MLCanvasSetPixel(canvas_diff, x, y, COLOR_DIFF);
                if (color_eql) {
                    MLColor gray = MLColorToGrayscale(color_A);
                    MLCanvasSetPixel(canvas_diff, x, y, gray);
                } else {
                    MLCanvasSetPixel(canvas_diff, x, y, COLOR_DIFF);
                }
            } else if (diff_mode == DIFF_MODE_BLEND_DIFF) {
                MLColor difference = MLColorDifferenceBlend(color_A, color_B);
                MLCanvasSetPixel(canvas_diff, x, y, difference);
            } else if (diff_mode == DIFF_MODE_OURS) {
                MLCanvasSetPixel(canvas_diff, x, y, color_A);
            } else {
                fprintf(stderr, "INVALID DIFF MODE: %d\n", diff_mode);
                exit(1);
            }
        }
    }

    return diff;
}

void TestMLCanvas(const MLCanvas *const canvas, const char *ref_file,
                  DiffMode diff_mode) {
    MLCanvas canvas_diff = {0};
    MLCanvas ref_canvas = MLCanvasLoadPPM(ref_file);
    if (MLCanvasDiff(canvas, &ref_canvas, &canvas_diff, diff_mode)) {
        char failed_name[64] = TEST_DIR "FAILED_";
        char ref[64];
        strcpy(ref, ref_file);
        char *token = strtok(ref, "/");
        token = strtok(NULL, "/");
        strcat(failed_name, token);
        MLCanvasRenderPPM(&canvas_diff, failed_name);
        ANSIEscapeStdout(ANSI_RED);
        printf("❌ TEST %s FAILED! Diff rendered to %s\n", ref_file,
               failed_name);
        ANSIEscapeStdout(ANSI_RESET);
    } else {
        ANSIEscapeStdout(ANSI_GREEN);
        printf("✅ TEST %s SUCCEEDED!\n", ref_file);
        ANSIEscapeStdout(ANSI_RESET);
    }

    MLCanvasDestroy(&ref_canvas);
    MLCanvasDestroy(&canvas_diff);
}

void TestCase(ExampleFn fn, const char *ref_file, Subcommand cmd,
              DiffMode diff_mode) {
    printf("Running test case %s\n", ref_file);

    // Set printf to gray for output collected during test
    ANSIEscapeStdout(ANSI_BRIGHT_BLACK);

    MLCanvas canvas = MLCanvasMake(WIDTH, HEIGHT, ML_COLOR_WHITE);
    fn(&canvas);

    if (cmd == CMD_RUN) {
        TestMLCanvas(&canvas, ref_file, diff_mode);
    } else if (cmd == CMD_REGISTER) {
        // Registers the test case as the new reference data.
        MLCanvasRenderPPM(&canvas, ref_file);
    }

    MLCanvasDestroy(&canvas);

    printf("\n");
}

void ShapesExample(MLCanvas *const canvas) {
    MLCanvasFillRect(canvas, 4, 4, WIDTH - 8, HEIGHT - 8, C(0xFFD9A403));
    MLCanvasFillCircle(canvas, 0, 0, HEIGHT / 5, C(0xBBC35DFA));
    MLCanvasFillCircle(canvas, -20, -20, HEIGHT / 8, C(0xBA00B4D8));
    MLCanvasFillQuad(canvas, MLPoint2DMake(75, 200), MLPoint2DMake(200, 20),
                     MLPoint2DMake(300, 275), MLPoint2DMake(500, 10),
                     C(0xFFFA0301));
    MLCanvasFillTriangle(canvas, 200, 200, 300, 200, 500, 400, C(0x881AB3FD));
}

void LinesExample(MLCanvas *const canvas) {
    MLCanvasDrawLine(canvas, 0, 0, WIDTH - 1, HEIGHT - 1, ML_COLOR_BLACK, 1);
    MLCanvasDrawLine(canvas, WIDTH - 1, 0, 0, HEIGHT - 1, ML_COLOR_BLACK, 1);
    MLCanvasDrawLine(canvas, WIDTH / 2, 0, WIDTH / 2, HEIGHT - 1,
                     ML_COLOR_BLACK, 1);
    MLCanvasDrawLine(canvas, 0, HEIGHT / 2, WIDTH - 1, HEIGHT / 2,
                     ML_COLOR_BLACK, 1);
    MLCanvasDrawLine(canvas, 0, 0, WIDTH - 1, HEIGHT / 2, ML_COLOR_BLACK, 1);
    MLCanvasDrawLine(canvas, WIDTH - 1, 0, 0, HEIGHT / 2, ML_COLOR_BLACK, 1);
    MLCanvasDrawLine(canvas, 0, HEIGHT - 1, WIDTH - 1, HEIGHT / 2,
                     ML_COLOR_BLACK, 1);
    MLCanvasDrawLine(canvas, WIDTH - 1, HEIGHT - 1, 0, HEIGHT / 2,
                     ML_COLOR_BLACK, 1);
}

void ThiccLinesExample(MLCanvas *const canvas) {
    uint32_t graph_origin_x = WIDTH / 2 - WIDTH / 4;

    // Some lines
    MLCanvasDrawLine(canvas, graph_origin_x, HEIGHT - 80,
                     HEIGHT - 160 + graph_origin_x - 100, 180, ML_COLOR_BLUE,
                     1);
    MLCanvasDrawLine(canvas, graph_origin_x, HEIGHT - 80,
                     HEIGHT - 160 + graph_origin_x - 200, 160, ML_COLOR_RED, 1);
    MLCanvasDrawLine(canvas, graph_origin_x, HEIGHT - 80,
                     HEIGHT - 160 + graph_origin_x - 40, HEIGHT - 80 - 20,
                     C(0xFF00DC00), 1);

    // Axes (draw last for Z reasons)
    MLCanvasDrawLine(canvas, graph_origin_x, 80, graph_origin_x, HEIGHT - 80,
                     ML_COLOR_BLACK, 3);
    MLCanvasDrawLine(canvas, graph_origin_x, HEIGHT - 80,
                     HEIGHT - 160 + graph_origin_x, HEIGHT - 80, ML_COLOR_BLACK,
                     3);

    // Labels
    const char *title = "Fruit Consumption vs All-cause Mortality";
    int title_sz = strlen(title);
    MLCanvasWriteString(canvas, title,
                        WIDTH / 2 - (title_sz * Mojangles.glyph_width) / 2,
                        HEIGHT - 60, Mojangles, 1, ML_COLOR_BLACK);
}

void TextExample(MLCanvas *const canvas) {
    MLCanvasWriteString(canvas, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 20, HEIGHT / 2,
                        Mojangles, 2, ML_COLOR_BLACK);
    MLCanvasWriteString(canvas, "abcdefghijklmnopqrstuvwxyz", 20,
                        HEIGHT / 2 + 20, Mojangles, 1, ML_COLOR_BLACK);
}

void TriangleExample(MLCanvas *const canvas) {
    MLCanvasFillTriangle(canvas, 0, 0, WIDTH / 2, 10, WIDTH / 2 - 10,
                         HEIGHT / 2, ML_COLOR_BLACK);
    MLCanvasFillTriangle(canvas, 30, 40, 200, 70, 4, 369, C(0xBA2308FF));
    MLCanvasFillTriangle(canvas, WIDTH / 2 - 90, HEIGHT / 2 - 20, 550,
                         HEIGHT / 2 - 90, 550, HEIGHT / 2 + 100, C(0xC3B8D908));
    MLCanvasWriteString(canvas, "THESE ARE TRIANGLES", 10, HEIGHT - 20,
                        Mojangles, 2, ML_COLOR_BLACK);
}

int main(int argc, char **argv) {
    nob_mkdir_if_not_exists(TEST_DIR);

    // Parse command. Default mode is CMD_TEST
    nob_shift(argv, argc);
    Subcommand cmd;
    DiffMode diff_mode = DIFF_MODE_GRAYSCALE;

    if (argc <= 0) {
        printf("ERROR: No command");
        exit(1);
    }
    const char *command = nob_shift(argv, argc);
    if (strcmp(command, "run") == 0) {
        cmd = CMD_RUN;

        // Parse difference mode
        const char *option = nob_shift(argv, argc);
        if (strcmp(option, "-mode") == 0) {
            const char *diff_mode_s = nob_shift(argv, argc);
            if (strcmp(diff_mode_s, "grayscale") == 0) {
                diff_mode = DIFF_MODE_GRAYSCALE;
            } else if (strcmp(diff_mode_s, "difference") == 0) {
                diff_mode = DIFF_MODE_BLEND_DIFF;
            } else if (strcmp(diff_mode_s, "ours") == 0) {
                diff_mode = DIFF_MODE_OURS;
            } else {
                fprintf(stderr, "ILLEGAL MODE: %s\n", diff_mode_s);
                exit(1);
            }

            printf("Using difference mode: %s\n", diff_mode_s);
        } else if (strlen(option) > 0) {
            printf("Illegal option %s for command 'run'", option);
            exit(1);
        }
    } else if (strcmp(command, "register") == 0) {
        cmd = CMD_REGISTER;
    } else {
        printf("Illegal subcommand: %s\n", command);
        exit(1);
    }

    TestCase(&ShapesExample, TEST_DIR "shapes.ppm", cmd, diff_mode);
    TestCase(&LinesExample, TEST_DIR "lines.ppm", cmd, diff_mode);
    TestCase(&ThiccLinesExample, TEST_DIR "thicc.ppm", cmd, diff_mode);
    TestCase(&TextExample, TEST_DIR "text.ppm", cmd, diff_mode);
    TestCase(&TriangleExample, TEST_DIR "tri.ppm", cmd, diff_mode);

    return 0;
}
