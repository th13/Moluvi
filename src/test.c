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

#define COLOR_DIFF COLOR_RED

enum ansi_esc {
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
};

void ansi_esc_stdout(int escape) { printf("\033[%dm", escape); }
//TODO: ENUM RENAMES
enum subcommand {
    CMD_RUN,
    CMD_REGISTER,
};

 enum diff_mode {
    DIFF_MODE_GRAYSCALE, // Render the output in grayscale, highlighting diff in
                         // red
    DIFF_MODE_OURS,      // Render our test output, with no diff compared to the
                         // original
    DIFF_MODE_BLEND_DIFF // Render the output using difference blending (i.e.
                         // |fg - bg|)
};

typedef void (*example_fn)(canvas_t *const);

bool canvas_calc_diff(const canvas_t *const canvas_A,
                  const canvas_t *const canvas_B, canvas_t *canvas_diff,
                  enum diff_mode diff_mode) {
    uint32_t diff_width = MAX(canvas_A->width, canvas_B->width);
    uint32_t diff_height = MAX(canvas_A->height, canvas_B->height);
    canvas_init(canvas_diff, diff_width, diff_height, COLOR_BLACK);

    bool diff = false;
    for (uint32_t x = 0; x < diff_width; x++) {
        for (uint32_t y = 0; y < diff_height; y++) {
            struct rgba color_A ;
            canvas_get_px(canvas_A, x, y, &color_A);
            struct rgba color_B ;
            canvas_get_px(canvas_B, x, y, &color_B);

            bool color_eql = rgba_eql(color_A, color_B);
            if (!color_eql)
                diff = true;

            if (diff_mode == DIFF_MODE_GRAYSCALE) {
                canvas_set_px(canvas_diff, x, y, COLOR_DIFF);
                if (color_eql) {
                    struct rgba gray = rgba_convert_grayscale(color_A);
                    canvas_set_px(canvas_diff, x, y, gray);
                } else {
                    canvas_set_px(canvas_diff, x, y, COLOR_DIFF);
                }
            } else if (diff_mode == DIFF_MODE_BLEND_DIFF) {
                struct rgba difference = rgba_diff_blend(color_A, color_B);
                canvas_set_px(canvas_diff, x, y, difference);
            } else if (diff_mode == DIFF_MODE_OURS) {
                canvas_set_px(canvas_diff, x, y, color_A);
            } else {
                fprintf(stderr, "INVALID DIFF MODE: %d\n", diff_mode);
                exit(1);
            }
        }
    }

    return diff;
}

void canvas_test_diff(const canvas_t *const canvas, const char *ref_file,
                  enum diff_mode diff_mode) {
    canvas_t canvas_diff = {0};
    canvas_t ref_canvas = canvas_load_ppm(ref_file);
    if (canvas_calc_diff(canvas, &ref_canvas, &canvas_diff, diff_mode)) {
        char failed_name[64] = TEST_DIR "FAILED_";
        char ref[64];
        strcpy(ref, ref_file);
        char *token = strtok(ref, "/");
        token = strtok(NULL, "/");
        strcat(failed_name, token);
        canvas_render_ppm(&canvas_diff, failed_name);
        ansi_esc_stdout(ANSI_RED);
        printf("❌ TEST %s FAILED! Diff rendered to %s\n", ref_file,
               failed_name);
        ansi_esc_stdout(ANSI_RESET);
    } else {
        ansi_esc_stdout(ANSI_GREEN);
        printf("✅ TEST %s SUCCEEDED!\n", ref_file);
        ansi_esc_stdout(ANSI_RESET);
    }

    canvas_cleanup(&ref_canvas);
    canvas_cleanup(&canvas_diff);
}

void test_case(example_fn fn, const char *ref_file, enum subcommand cmd,
              enum diff_mode diff_mode) {
    printf("Running test case %s\n", ref_file);

    // Set printf to gray for output collected during test
    ansi_esc_stdout(ANSI_BRIGHT_BLACK);

    canvas_t canvas;
    canvas_init(&canvas, WIDTH, HEIGHT, COLOR_WHITE);
    fn(&canvas);

    if (cmd == CMD_RUN) {
        canvas_test_diff(&canvas, ref_file, diff_mode);
    } else if (cmd == CMD_REGISTER) {
        // Registers the test case as the new reference data.
        canvas_render_ppm(&canvas, ref_file);
    }

    canvas_cleanup(&canvas);

    printf("\n");
}

void shapes_example(canvas_t *const canvas) {
    canvas_fill_rect(canvas, 4, 4, WIDTH - 8, HEIGHT - 8, C(0xFFD9A403));
    canvas_fill_circle(canvas, 0, 0, HEIGHT / 5, C(0xBBC35DFA));
    canvas_fill_circle(canvas, -20, -20, HEIGHT / 8, C(0xBA00B4D8));
    canvas_fill_quad(canvas, (point2_t){75, 200}, (point2_t){200, 20},
                     (point2_t){300, 275}, (point2_t){500, 10},
                     C(0xFFFA0301));
    canvas_fill_tri(canvas, 200, 200, 300, 200, 500, 400, C(0x881AB3FD));
}

void lines_example(canvas_t *const canvas) {
    canvas_draw_line(canvas, 0, 0, WIDTH - 1, HEIGHT - 1, COLOR_BLACK, 1);
    canvas_draw_line(canvas, WIDTH - 1, 0, 0, HEIGHT - 1, COLOR_BLACK, 1);
    canvas_draw_line(canvas, WIDTH / 2, 0, WIDTH / 2, HEIGHT - 1,
                     COLOR_BLACK, 1);
    canvas_draw_line(canvas, 0, HEIGHT / 2, WIDTH - 1, HEIGHT / 2,
                     COLOR_BLACK, 1);
    canvas_draw_line(canvas, 0, 0, WIDTH - 1, HEIGHT / 2, COLOR_BLACK, 1);
    canvas_draw_line(canvas, WIDTH - 1, 0, 0, HEIGHT / 2, COLOR_BLACK, 1);
    canvas_draw_line(canvas, 0, HEIGHT - 1, WIDTH - 1, HEIGHT / 2,
                     COLOR_BLACK, 1);
    canvas_draw_line(canvas, WIDTH - 1, HEIGHT - 1, 0, HEIGHT / 2,
                     COLOR_BLACK, 1);
}

void thicc_lines_example(canvas_t *const canvas) {
    uint32_t graph_origin_x = WIDTH / 2 - WIDTH / 4;

    // Some lines
    canvas_draw_line(canvas, graph_origin_x, HEIGHT - 80,
                     HEIGHT - 160 + graph_origin_x - 100, 180, COLOR_BLUE,
                     1);
    canvas_draw_line(canvas, graph_origin_x, HEIGHT - 80,
                     HEIGHT - 160 + graph_origin_x - 200, 160, COLOR_RED, 1);
    canvas_draw_line(canvas, graph_origin_x, HEIGHT - 80,
                     HEIGHT - 160 + graph_origin_x - 40, HEIGHT - 80 - 20,
                     C(0xFF00DC00), 1);

    // Axes (draw last for Z reasons)
    canvas_draw_line(canvas, graph_origin_x, 80, graph_origin_x, HEIGHT - 80,
                     COLOR_BLACK, 3);
    canvas_draw_line(canvas, graph_origin_x, HEIGHT - 80,
                     HEIGHT - 160 + graph_origin_x, HEIGHT - 80, COLOR_BLACK,
                     3);

    // Labels
    const char *title = "Fruit Consumption vs All-cause Mortality";
    int title_sz = strlen(title);
    canvas_write_string(canvas, title,
                        WIDTH / 2 - (title_sz * font_mojangles.glyph_width) / 2,
                        HEIGHT - 60, font_mojangles, 1, COLOR_BLACK);
}

void text_example(canvas_t *const canvas) {
    canvas_write_string(canvas, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 20, HEIGHT / 2,
                        font_mojangles, 2, COLOR_BLACK);
    canvas_write_string(canvas, "abcdefghijklmnopqrstuvwxyz", 20,
                        HEIGHT / 2 + 20, font_mojangles, 1, COLOR_BLACK);
}

void triangle_example(canvas_t *const canvas) {
    canvas_fill_tri(canvas, 0, 0, WIDTH / 2, 10, WIDTH / 2 - 10,
                         HEIGHT / 2, COLOR_BLACK);
    canvas_fill_tri(canvas, 30, 40, 200, 70, 4, 369, C(0xBA2308FF));
    canvas_fill_tri(canvas, WIDTH / 2 - 90, HEIGHT / 2 - 20, 550,
                         HEIGHT / 2 - 90, 550, HEIGHT / 2 + 100, C(0xC3B8D908));
    canvas_write_string(canvas, "THESE ARE TRIANGLES", 10, HEIGHT - 20,
                        font_mojangles, 2, COLOR_BLACK);
}

int main(int argc, char **argv) {
    nob_mkdir_if_not_exists(TEST_DIR);

    // Parse command. Default mode is CMD_TEST
    nob_shift(argv, argc);
    enum subcommand cmd;
    enum diff_mode diff_mode = DIFF_MODE_GRAYSCALE;

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

    test_case(&shapes_example, TEST_DIR "shapes.ppm", cmd, diff_mode);
    test_case(&lines_example, TEST_DIR "lines.ppm", cmd, diff_mode);
    test_case(&thicc_lines_example, TEST_DIR "thicc.ppm", cmd, diff_mode);
    test_case(&text_example, TEST_DIR "text.ppm", cmd, diff_mode);
    test_case(&triangle_example, TEST_DIR "tri.ppm", cmd, diff_mode);

    return 0;
}
