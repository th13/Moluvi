#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Math utilities */
#define MIN(a, b) (a) < (b) ? (a) : (b)
#define MAX(a, b) (a) > (b) ? (a) : (b)
#define SUB_SATURATED(a, b) ((a) > (b) ? (a - b) : 0)
#define IN_IRANGEF(x, min, max, eps)                                           \
    ((x) >= (min) - (eps) && (x) <= (max) + (eps))
#define IN_XRANGE(x, min, max) ((x) >= (min) && (x) <= (max))

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
    return (uint8_t)(((uint16_t)bg * (255 - a) + (uint16_t)fg * a) / 255);
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

RGBA RGBAToGrayscale(RGBA color) {
    uint32_t gray = (299 * color.r + 587 * color.g + 114 * color.b) / 1000;
    gray = MIN(255, gray);
    return (RGBA){.r = (uint8_t)gray, .g = (uint8_t)gray, .b = (uint8_t)gray};
}

uint32_t ColorToGrayscale(uint32_t color) {
    return RGBAToHex(RGBAToGrayscale(RGBAFromHex(color)));
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
    for (uint32_t i = 0; i < width * height; i++) {
        data[i] = fill;
    }
    return (Canvas){.width = width, .height = height, .data = data};
}

void CanvasFreeData(Canvas *canvas) {
    if (!canvas)
        return;
    free(canvas->data);
}

uint32_t CanvasGetPixel(const Canvas *const canvas, uint32_t x, uint32_t y) {
    assert(x >= 0 && x < canvas->width && y >= 0 && y < canvas->height);
    return canvas->data[y * canvas->width + x];
}

/**
 * Sets pixel at point (x, y) to the specified color.
 */
void CanvasSetPixel(Canvas *const canvas, uint32_t x, uint32_t y,
                    uint32_t color) {
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
void CanvasFillTriangle(Canvas *const canvas, int64_t x0, int64_t y0,
                        int64_t x1, int64_t y1, int64_t x2, int64_t y2,
                        uint32_t color) {
    // Bounding region
    int64_t start_x = MIN(MIN(x0, x1), x2);
    int64_t start_y = MIN(MIN(y0, y1), y2);
    int64_t end_x = MAX(MAX(x0, x1), x2);
    int64_t end_y = MAX(MAX(y0, y1), y2);
    assert(start_x >= 0 && start_y >= 0 && end_x < canvas->width &&
           end_y < canvas->height);

    int64_t dy01 = y0 - y1;
    int64_t dy12 = y1 - y2;
    int64_t dy20 = y2 - y0;
    int64_t area = x0 * dy12 + x1 * dy20 + x2 * dy01;
    assert(area != 0); // no degenerate triangles

    for (int64_t ix = start_x; ix <= end_x; ix++) {
        for (int64_t iy = start_y; iy <= end_y; iy++) {

            int64_t area_p12 = ix * dy12 + x1 * (y2 - iy) + x2 * (iy - y1);
            int64_t area_p02 = ix * dy20 + x2 * (y0 - iy) + x0 * (iy - y2);
            int64_t area_p01 = ix * dy01 + x0 * (y1 - iy) + x1 * (iy - y0);
            float u = (float)area_p12 / area;
            float v = (float)area_p02 / area;
            float w = (float)area_p01 / area;
            if (IN_IRANGEF(u, 0, 1, 1e-3) && IN_IRANGEF(v, 0, 1, 1e-3) &&
                IN_IRANGEF(w, 0, 1, 1e-3)) {
                CanvasBlendPixel(canvas, ix, iy, color);
            }
        }
    }
}

/**
 * Draws a line from point (x0, y0) to (x1, y1) of width 1.
 *
 * @todo Line thickness
 * @todo Antialiasing
 */
void CanvasDrawLine(Canvas *const canvas, uint32_t x0, uint32_t y0, uint32_t x1,
                    uint32_t y1, uint32_t color, uint32_t thiccness) {
    uint32_t start_x = x0;
    uint32_t start_y = y0;
    uint32_t end_x = x1;
    uint32_t end_y = y1;

    if (start_x == end_x) {
        if (start_y > end_y)
            SWAP(uint32_t, start_y, end_y);

        for (uint32_t iy = start_y; iy <= end_y; iy++) {
            for (uint32_t x = start_x; x < thiccness + start_x; x++) {
                if (IN_XRANGE(x, 0, canvas->width) &&
                    IN_XRANGE(iy, 0, canvas->height)) {
                    CanvasBlendPixel(canvas, x, iy, color);
                }
            }
        }

        return;
    }

    if (start_x > end_x) {
        SWAP(uint32_t, start_x, end_x);
        SWAP(uint32_t, start_y, end_y);
    }

    float dy = (float)start_y - (float)end_y;
    float dx = (float)start_x - (float)end_x;
    float m = dy / dx;
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
            for (uint32_t x_thicc = ix; x_thicc < ix + thiccness; x_thicc++) {

                if (iy < canvas->height && x_thicc < canvas->width) {
                    CanvasBlendPixel(canvas, x_thicc, iy, color);
                }
            }
        }

        for (uint32_t y_thicc = ry0; y_thicc < ry0 + thiccness; y_thicc++) {
            if (y_thicc < canvas->height && ix < canvas->width) {
                CanvasBlendPixel(canvas, ix, y_thicc, color);
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

/* Font: Mojangles */

#define MJ_GLYPH_WIDTH 8
#define MJ_GLYPH_HEIGHT 8
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
    },
    ['a'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,1,1,1,0,0,0,
        0,0,0,0,0,1,0,0,
        0,0,1,1,1,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['b'] = {
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,1,1,0,0,0,
        0,1,1,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['c'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['d'] = {
        0,0,0,0,0,1,0,0,
        0,0,0,0,0,1,0,0,
        0,0,1,1,0,1,0,0,
        0,1,0,0,1,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['e'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,1,1,1,1,0,0,
        0,1,0,0,0,0,0,0,
        0,0,1,1,1,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['f'] = {
        0,0,0,1,1,0,0,0,
        0,0,1,0,0,0,0,0,
        0,1,1,1,1,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['g'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,1,1,1,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,1,0,0,
        0,0,0,0,0,1,0,0,
        0,1,1,1,1,0,0,0,
    },
    ['h'] = {
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,1,1,0,0,0,
        0,1,1,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['i'] = {
        0,0,1,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['j'] = {
        0,0,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,1,0,0,
        0,0,0,0,0,1,0,0,
        0,0,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['k'] = {
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,1,0,0,0,
        0,1,0,1,0,0,0,0,
        0,1,1,0,0,0,0,0,
        0,1,0,1,0,0,0,0,
        0,1,0,0,1,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['l'] = {
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['m'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,1,1,0,1,0,0,0,
        0,1,0,1,0,1,0,0,
        0,1,0,1,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['n'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,1,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['o'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,1,1,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['p'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,1,0,1,1,0,0,0,
        0,1,1,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,1,1,1,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
    },
    ['q'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,1,1,0,1,0,0,
        0,1,0,0,1,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,1,0,0,
        0,0,0,0,0,1,0,0,
        0,0,0,0,0,1,0,0,
    },
    ['r'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,1,0,1,1,0,0,0,
        0,1,1,0,0,1,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,1,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['s'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,1,1,1,1,0,0,
        0,1,0,0,0,0,0,0,
        0,0,1,1,1,0,0,0,
        0,0,0,0,0,1,0,0,
        0,1,1,1,1,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['t'] = {
        0,0,1,0,0,0,0,0,
        0,1,1,1,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['u'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['v'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,0,1,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['w'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,1,0,1,0,0,
        0,1,0,1,0,1,0,0,
        0,0,1,1,1,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['x'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,0,1,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,1,0,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,0,0,0,0,0,0,0,
    },
    ['y'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,1,1,1,0,0,
        0,0,0,0,0,1,0,0,
        0,1,1,1,1,0,0,0,
    },
    ['z'] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,1,1,1,1,1,0,0,
        0,0,0,0,1,0,0,0,
        0,0,0,1,0,0,0,0,
        0,0,1,0,0,0,0,0,
        0,1,1,1,1,1,0,0,
        0,0,0,0,0,0,0,0,
    },

};
// clang-format on

static Font Mojangles = {
    .glyph_width = MJ_GLYPH_WIDTH,
    .glyph_height = MJ_GLYPH_HEIGHT,
    .glyphs = &GlyphsMojangles[0][0],
};

/**
 * Draws a character at point (x, y).
 */
void CanvasDrawChar(Canvas *const canvas, char c, uint32_t x, uint32_t y,
                    Font font, uint32_t font_size) {
    const char *glyph = FontGetGlyph(font, c);
    uint32_t end_x = x + font_size * font.glyph_width - 1;
    uint32_t end_y = y + font_size * font.glyph_height - 1;
    assert(end_x < canvas->width && end_y < canvas->height);

    for (uint32_t ix = x; ix <= end_x; ix++) {
        for (uint32_t iy = y; iy <= end_y; iy++) {
            uint32_t i_glyph = ((iy - y) / font_size) * font.glyph_width +
                               ((ix - x) / font_size);
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
                       uint32_t y, Font font, uint32_t font_size) {
    uint32_t len = strlen(str);
    if (len == 0)
        return;
    assert(x + len * font.glyph_width < canvas->width &&
           y + font.glyph_height < canvas->height);

    for (uint32_t i = 0; i < len; i++) {
        char c = str[i];
        uint32_t ix = x + i * font_size * font.glyph_width;
        CanvasDrawChar(canvas, c, ix, y, font, font_size);
    }
}

/**
 * Renders a canvas to a file in PPM P6 format.
 */
void CanvasRenderPPM(const Canvas *const canvas, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Could not open file");
        return;
    }

    fprintf(file, "P6\n%u %u 255\n", canvas->width, canvas->height);
    for (uint32_t i = 0; i < canvas->width * canvas->height; i++) {
        RGBA rgba = RGBAFromHex(canvas->data[i]);
        fwrite(&rgba.r, sizeof(uint8_t), 1, file);
        fwrite(&rgba.g, sizeof(uint8_t), 1, file);
        fwrite(&rgba.b, sizeof(uint8_t), 1, file);
    }
    fclose(file);
}

/**
 * Loads a canvas from PPM P6 file.
 */
Canvas CanvasLoadPPM(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Could not load canvas");
        exit(1);
    }

    char *token = NULL;

    // Parse format
    char line[32];
    if (!fgets(line, sizeof(line), file)) {
        perror("Could not read format from file");
        exit(1);
    }
    printf("LOADING CANVAS FROM FILE %s, FORMAT=%s", filename, line);

    token = strtok(line, "\n");
    if (strcmp(token, "P6") != 0) {
        perror("CanvasLoadPPM currently only supports P6 format\n");
        exit(1);
    }

    // Parse size and (ignored for now) color depth
    uint32_t width;
    uint32_t height;
    uint16_t bit_depth;

    if (!fgets(line, sizeof(line), file)) {
        perror("Could not read size/color depth from file");
        exit(1);
    }

    token = strtok(line, " ");
    width = atoi(token);
    token = strtok(NULL, " ");
    height = atoi(token);
    token = strtok(NULL, " ");
    bit_depth = atoi(token);
    printf("CANVAS WIDTH=%u, HEIGHT=%u, BIT_DEPTH=%u\n", width, height,
           bit_depth);

    // Interpret bytes as colors (uint32_t)
    uint32_t *data = (uint32_t *)malloc(sizeof(uint32_t) * width * height);
    if (!data) {
        perror("Could not allocate data\n");
        exit(1);
    }
    char *raw_data = (char *)malloc(3 * width * height);
    if (!raw_data) {
        perror("Could not allocate raw data\n");
        exit(1);
    }

    size_t bytes_read = fread(raw_data, 1, 3 * width * height, file);
    if (bytes_read != 3 * width * height) {
        perror("Could not read enough bytes\n");
        printf("Bytes read %lu\n", bytes_read);
        exit(1);
    }

    for (uint32_t i = 0; i < width * height; i++) {
        RGBA rgba = {0};
        rgba.r = raw_data[i * 3];
        rgba.g = raw_data[i * 3 + 1];
        rgba.b = raw_data[i * 3 + 2];
        rgba.a = 0xFF;
        data[i] = RGBAToHex(rgba);
    }

    fclose(file);

    return (Canvas){.width = width, .height = height, .data = data};
}
