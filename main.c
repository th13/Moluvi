#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// @todo Blend alphas

/* Math utilities */
#define MIN(a, b) (a) < (b) ? (a) : (b)
#define MAX(a, b) (a) > (b) ? (a) : (b)
#define SUB_SATURATED(a, b) ((a) > (b) ? (a - b) : 0)

/* Memory/misc utilities */
#define SWAP(T, a, b)                                                          \
    do {                                                                       \
        T tmp = a;                                                             \
        a = b;                                                                 \
        b = tmp;                                                               \
    } while (0);

/* Color definitions */
#define COLOR_BLACK 0x000000FF
#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_RED 0xFF0000FF
#define COLOR_GREEN 0x00FF00FF
#define COLOR_BLUE 0x0000FFFF

typedef struct RGBA {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} RGBA;

uint8_t blendComponent(uint8_t bg, uint8_t fg, uint8_t a) {
    return (uint8_t)((uint16_t)(bg - bg * a / 255) + (uint16_t)(fg * a / 255));
}

/**
 * Creates a color (hex) from RGBA components.
 */
uint32_t RGBAToHex(RGBA color) {
    uint32_t hex = color.a;
    hex |= (uint32_t)color.b << 8;
    hex |= (uint32_t)color.g << 16;
    hex |= (uint32_t)color.r << 24;
    return hex;
}

RGBA RGBAFromHex(uint32_t hex) {
    RGBA color = {0};
    color.a = (uint8_t)(hex & 0xFF);
    color.b = (uint8_t)((hex >> 8) & 0xFF);
    color.g = (uint8_t)((hex >> 16) & 0xFF);
    color.r = (uint8_t)((hex >> 24) & 0xFF);
    return color;
}

uint32_t ColorBlend(uint32_t fg, uint32_t bg) {
    RGBA fgc = RGBAFromHex(fg);
    RGBA bgc = RGBAFromHex(bg);
    fgc.r = blendComponent(bgc.r, fgc.r, fgc.a);
    fgc.g = blendComponent(bgc.g, fgc.g, fgc.a);
    fgc.b = blendComponent(bgc.b, fgc.b, fgc.a);
    fgc.a = 0xFF;
    return RGBAToHex(fgc);
}

/**
 * Represents a virtual canvas of size width x height.
 * Contains a pointer to a color array expected to be of equal size.
 */
typedef struct Canvas {
    uint32_t width;
    uint32_t height;
    uint32_t *data;
} Canvas;

/**
 * Makes a new canvas of size width x height with initial fill color.
 * THIS METHOD ALLOCATES ITEMS IN THE Canvas STRUCT!
 */
Canvas MakeCanvas(uint32_t width, uint32_t height, uint32_t fill) {
    uint32_t *data = (uint32_t *)malloc(width * height * sizeof(uint32_t));
    for (int i = 0; i < width * height; i++) {
        data[i] = fill;
    }
    return (Canvas){.width = width, .height = height, .data = data};
}

void CanvasDestroy(Canvas *canvas) { free(canvas->data); }

uint32_t CanvasGetPixel(const Canvas *const canvas, uint32_t x, uint32_t y) {
    assert(x >= 0 && x < canvas->width && y >= 0 && y < canvas->height);
    return canvas->data[y * canvas->width + x];
}

/**
 * Sets pixel at point (x, y) to the specified color.
 */
void CanvasSetPixel(Canvas *const canvas, uint32_t x, uint32_t y,
                    uint32_t color) {
    uint32_t bg = CanvasGetPixel(canvas, x, y);
    canvas->data[y * canvas->width + x] = color;
}

void CanvasBlendPixel(Canvas *const canvas, uint32_t x, uint32_t y,
                      uint32_t color) {
    uint32_t bg = CanvasGetPixel(canvas, x, y);
    uint32_t blend = ColorBlend(color, bg);
    CanvasSetPixel(canvas, x, y, blend);
}

/**
 * Fills a rectangle in the canvas of size width x height with top left corner
 * at point (x, y).
 */
void CanvasFillRect(Canvas *const canvas, uint32_t x, uint32_t y,
                    uint32_t width, uint32_t height, uint32_t color) {
    for (uint32_t iy = y; iy < height + y; iy++) {
        for (uint32_t ix = x; ix < width + x; ix++) {
            CanvasBlendPixel(canvas, ix, iy, color);
        }
    }
}

/**
 * Fills a circle in the canvas centered at point (cx, cy) with radius r.
 * @todo Antialiasing
 */
void CanvasFillCircle(Canvas *const canvas, uint32_t center_x,
                      uint32_t center_y, uint32_t radius, uint32_t color) {
    uint32_t start_x = SUB_SATURATED(center_x, radius);
    uint32_t start_y = SUB_SATURATED(center_y, radius);
    uint32_t end_x = MIN(canvas->width - 1, center_x + radius);
    uint32_t end_y = MIN(canvas->height - 1, center_y + radius);

    for (uint32_t ix = start_x; ix <= end_x; ix++) {
        for (uint32_t iy = start_y; iy <= end_y; iy++) {
            int64_t offset_x = (int64_t)ix - (int64_t)center_x;
            int64_t offset_y = (int64_t)iy - (int64_t)center_y;
            if (offset_x * offset_x + offset_y * offset_y <= radius * radius) {
                CanvasBlendPixel(canvas, ix, iy, color);
            }
        }
    }
}

/**
 * Fills a triangle in the canvas based on 3 points.
 */
void CanvasFillTriangle(Canvas *const canvas, uint32_t x0, uint32_t y0,
                        uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2,
                        uint32_t color) {}

/**
 * Draws a line from point (x0, y0) to (x1, y1) of width 1.
 *
 * @todo Line thickness
 * @todo Antialiasing
 */
void CanvasDrawLine(Canvas *const canvas, uint32_t x0, uint32_t y0, uint32_t x1,
                    uint32_t y1, uint32_t color) {
    uint32_t start_x = x0;
    uint32_t start_y = y0;
    uint32_t end_x = x1;
    uint32_t end_y = y1;

    if (start_x > end_x) {
        SWAP(uint32_t, start_x, end_x);
        SWAP(uint32_t, start_y, end_y);
    }

    float dy = (float)start_y - (float)end_y;
    float dx = (float)start_x - (float)end_x;
    float m = dx == 0 ? canvas->height * 2 : dy / dx;
    float b = (float)start_y - m * (float)start_x;

    for (uint32_t ix = start_x; ix <= end_x; ix++) {
        float real_y = m * (float)ix + b;
        uint32_t ry0 = (uint32_t)(real_y - (m / 2.));
        uint32_t ry1 = (uint32_t)(real_y + (m / 2.));

        if (ry0 > ry1) {
            SWAP(uint32_t, ry0, ry1);
        }

        for (uint32_t iy = ry0; iy <= ry1 || (iy < 0 && iy >= canvas->height);
             iy++) {
            if (iy < canvas->height && ix < canvas->width) {
                CanvasBlendPixel(canvas, ix, iy, color);
            }
        }
    }
}

/* Text */

typedef struct Font {
    uint32_t glyph_width;
    uint32_t glyph_height;
    const char *glyphs;
} Font;

const char *FontGetGlyph(Font font, char c) {
    return &font.glyphs[c * sizeof(char) * font.glyph_width *
                        font.glyph_height];
}

#define MJ_GLYPH_WIDTH 8
#define MJ_GLYPH_HEIGHT 9
#define MJ_GLYPH_SIZE (MJ_GLYPH_WIDTH * MJ_GLYPH_HEIGHT)

// clang-format off
const char GlyphsMojangles[128][MJ_GLYPH_SIZE] = {
    [' '] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['A'] = {
        0,0,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,1,1,1,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['B'] = {
        0,1,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['C'] = {
        0,0,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['D'] = {
        0,1,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['E'] = {
        0,1,1,1,1,1,0,0,
        0,1,0,0,0,0,0,0,
        0,1,1,1,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,1,1,1,1,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['F'] = {
        0,1,1,1,1,1,0,0,
        0,1,0,0,0,0,0,0,
        0,1,1,1,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['G'] = {
        0,0,1,1,1,1,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,1,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['H'] = {
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,1,1,1,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['I'] = {
        0,1,1,1,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,1,1,1,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['J'] = {
        0,0,0,0,0,1,0,0,
        0,0,0,0,0,1,0,0,
        0,0,0,0,0,1,0,0,
        0,0,0,0,0,1,0,0,
        0,0,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['K'] = {
        0,1,0,0,0,1,0,0,
        0,1,0,0,1,0,0,0,
        0,1,1,1,0,0,0,0,
        0,1,0,0,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['L'] = {
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,1,1,1,1,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['M'] = {
        0,1,0,0,0,1,0,0,
        0,1,1,0,1,1,0,0,
        0,1,0,1,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['N'] = {
        0,1,0,0,0,1,0,0,
        0,1,1,0,0,1,0,0,
        0,1,0,1,0,1,0,0,
        0,1,0,0,1,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['O'] = {
        0,0,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['P'] = {
        0,1,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,1,1,1,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['Q'] = {
        0,0,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,1,0,0,0,
        0,0,1,1,0,1,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['R'] = {
        0,1,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['S'] = {
        0,0,1,1,1,1,0,0,
        0,1,0,0,0,0,0,0,
        0,0,1,1,1,0,0,0,
        0,0,0,0,0,1,0,0,
        0,0,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['T'] = {
        0,1,1,1,1,1,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['U'] = {
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['V'] = {
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,0,1,0,0,0,
        0,0,1,0,1,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['W'] = {
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,1,0,1,0,0,
        0,1,1,0,1,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['X'] = {
        0,1,0,0,0,1,0,0,
        0,0,1,0,1,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,1,0,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['Y'] = {
        0,1,0,0,0,1,0,0,
        0,0,1,0,1,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['Z'] = {
        0,1,1,1,1,1,0,0,
        0,0,0,0,0,1,0,0,
        0,0,0,0,1,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,1,1,1,1,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },

};
// clang-format on

static Font Mojangles = {
    .glyph_width = 8,
    .glyph_height = 9,
    .glyphs = &GlyphsMojangles[0][0],
};

/**
 * Draws a character at point (x, y).
 * @todo Font and Glyph types.
 */
void CanvasDrawChar(Canvas *const canvas, char c, uint32_t x, uint32_t y,
                    Font font) {
    const char *glyph = FontGetGlyph(font, c);
    uint32_t end_x = x + font.glyph_width - 1;
    uint32_t end_y = y + font.glyph_height - 1;
    assert(end_x < canvas->width && end_y < canvas->height);

    for (uint32_t ix = x; ix <= end_x; ix++) {
        for (uint32_t iy = y; iy <= end_y; iy++) {
            uint32_t i_glyph = (iy - y) * font.glyph_width + (ix - x);
            if (glyph[i_glyph] == 1) {
                CanvasBlendPixel(canvas, ix, iy, COLOR_BLACK);
            }
        }
    }
}

/**
 * Writes text to the canvas starting at point (x, y).
 */
void CanvasWriteString(Canvas *const canvas, const char *str, uint32_t x,
                       uint32_t y, Font font) {
    uint32_t len = strlen(str);
    if (len == 0)
        return;
    assert(x + len * font.glyph_width < canvas->width &&
           y + font.glyph_height < canvas->height);

    for (int i = 0; i < len; i++) {
        char c = str[i];
        uint32_t ix = x + i * font.glyph_width;
        CanvasDrawChar(canvas, c, ix, y, font);
    }
}

/**
 * Renders a canvas to a file in PPM P6 format.
 */
void RenderCanvasP6(const char *filename, const Canvas *const canvas) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Could not open file");
        return;
    }

    fprintf(file, "P6\n%u %u 255\n", canvas->width, canvas->height);
    for (int i = 0; i < canvas->width * canvas->height; i++) {
        uint8_t *rgb = (uint8_t *)&canvas->data[i];
        fwrite(rgb + 1, sizeof(uint8_t), 3, file); // assumes little endian
    }
}

#define SCALE 10
#define WIDTH 64 * SCALE
#define HEIGHT 48 * SCALE

int main() {
    Canvas canvas = MakeCanvas(WIDTH, HEIGHT, 0xFF8732FF);
    CanvasFillRect(&canvas, 4, 4, WIDTH - 8, HEIGHT - 8, 0x03A4D9FF);
    CanvasFillCircle(&canvas, 0, 0, HEIGHT / 5, 0xFA5DC3BB);
    RenderCanvasP6("test.ppm", &canvas);
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
    RenderCanvasP6("lines.ppm", &canvas);
    CanvasDestroy(&canvas);

    canvas = MakeCanvas(WIDTH, HEIGHT, COLOR_WHITE);
    CanvasWriteString(&canvas, "FUCK YOU SPENSER", WIDTH / 2, HEIGHT / 2,
                      Mojangles);
    RenderCanvasP6("text.ppm", &canvas);
    CanvasDestroy(&canvas);
    return 0;
}
