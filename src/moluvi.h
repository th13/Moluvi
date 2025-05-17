#ifndef MOLUVI_H
#define MOLUVI_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//--------------------------------------------------------------------------------
// Types
//--------------------------------------------------------------------------------

typedef struct MLPoint2D {
    int64_t x;
    int64_t y;
} MLPoint2D;

typedef struct MLPoint3D {
    float x;
    float y;
    float z;
} MLPoint3D;

typedef struct MLColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} MLColor;

typedef struct MLCanvas {
    uint32_t width;
    uint32_t height;
    MLColor *data;
} MLCanvas;

typedef struct MLFont {
    uint32_t glyph_width;
    uint32_t glyph_height;
    const char *glyphs;
} MLFont;

//--------------------------------------------------------------------------------
// API
//--------------------------------------------------------------------------------

// Math utilities
#ifndef MIN
#define MIN(a, b) (a) < (b) ? (a) : (b)
#endif

#ifndef MAX
#define MAX(a, b) (a) > (b) ? (a) : (b)
#endif

#define SUB_SATURATED(a, b) ((a) > (b) ? (a - b) : 0)
#define IN_IRANGEF(x, min, max, eps)                                           \
    ((x) >= (min) - (eps) && (x) <= (max) + (eps))
#define IN_XRANGE(x, min, max) ((x) >= (min) && (x) <= (max))

// Memory/misc utilities
#ifndef SWAP
#define SWAP(T, a, b)                                                          \
    do {                                                                       \
        T tmp = a;                                                             \
        a = b;                                                                 \
        b = tmp;                                                               \
    } while (0);
#endif

// Common color definitions
#define ML_COLOR_BLACK (MLColor){0, 0, 0, 255}
#define ML_COLOR_WHITE (MLColor){255, 255, 255, 255}
#define ML_COLOR_RED (MLColor){255, 0, 0, 255}
#define ML_COLOR_GREEN (MLColor){0, 255, 0, 255}
#define ML_COLOR_BLUE (MLColor){0, 0, 255, 255}

#ifndef C
#define C(hex) MLColorFromHex(hex)
#else
#define COLOR(hex) MLColorfroMLColorFromHex(hex)
#endif

// MLCanvas creation & utils
MLCanvas MLCanvasMake(uint32_t width, uint32_t height, MLColor fill);
void MLCanvasDestroy(MLCanvas *canvas);
MLColor MLCanvasGetPixel(const MLCanvas *const canvas, uint32_t x, uint32_t y);
void MLCanvasSetPixel(MLCanvas *const canvas, uint32_t x, uint32_t y,
                      MLColor color);
void MLCanvasBlendPixel(MLCanvas *const canvas, uint32_t x, uint32_t y,
                        MLColor color);

// MLPoint utils
MLPoint2D MLPoint2DMake(int64_t x, int64_t y);

// MLColor functions
uint32_t MLColorToHex(MLColor color);
MLColor MLColorFromHex(uint32_t hex);
MLColor MLColorToGrayscale(MLColor color);
MLColor MLColorBlend(MLColor fg, MLColor bg);
bool MLColorEqual(MLColor c1, MLColor c2);
MLColor MLColorDifferenceBlend(MLColor c1, MLColor c2);

// MLCanvas fills & draws
void MLCanvasFill(MLCanvas *const canvas, MLColor color);
void MLCanvasFillRect(MLCanvas *const canvas, uint32_t x, uint32_t y,
                      uint32_t width, uint32_t height, MLColor color);
void MLCanvasFillCircle(MLCanvas *const canvas, int64_t center_x,
                        int64_t center_y, uint32_t radius, MLColor color);
void MLCanvasFillTriangle(MLCanvas *const canvas, int64_t x0, int64_t y0,
                          int64_t x1, int64_t y1, int64_t x2, int64_t y2,
                          MLColor color);
void MLCanvasFillQuad(MLCanvas *const canvas, MLPoint2D p1, MLPoint2D p2,
                      MLPoint2D p3, MLPoint2D p4, MLColor color);
void MLCanvasDrawLine(MLCanvas *const canvas, uint32_t x0, uint32_t y0,
                      uint32_t x1, uint32_t y1, MLColor color,
                      uint32_t thiccness);

// Text drawing
void MLCanvasDrawChar(MLCanvas *const canvas, char c, uint32_t x, uint32_t y,
                      MLFont font, uint32_t font_size, MLColor color);
void MLCanvasWriteString(MLCanvas *const canvas, const char *str, uint32_t x,
                         uint32_t y, MLFont font, uint32_t font_size,
                         MLColor color);

// MLCanvas rendering
void MLCanvasRenderPPM(const MLCanvas *const canvas, const char *filename);
MLCanvas MLCanvasLoadPPM(const char *filename);

// MLFont utils
const char *MLFontGetGlyph(MLFont font, char c);

// Misc utilities
float lerpf(float t, float a, float b);
double lerpd(double t, double a, double b);

#endif // MOLUVI_H
