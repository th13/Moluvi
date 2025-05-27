#ifndef MOLUVI_H
#define MOLUVI_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//--------------------------------------------------------------------------------
// Types
//--------------------------------------------------------------------------------

typedef struct MLVector2Int64 {
    int64_t x;
    int64_t y;
} MLVector2Int64;

typedef struct MLVector3Float {
    float x;
    float y;
    float z;
} MLVector3Float;

typedef struct MLVector3Int64 {
    int64_t x;
    int64_t y;
    int64_t z;
} MLVector3Int64;

typedef struct MLVector3Size {
    size_t x;
    size_t y;
    size_t z;
} MLVector3Size;

#define MLPoint2D MLVector2Int64
#define MLPoint3D MLVector3Float

typedef struct MLCamera {
    float focal_len; // The focal length of the camera
    float dist;      // The distance of camera from screen
    uint32_t width;  // The screen width
    uint32_t height; // The screen height
} MLCamera;

typedef struct MLColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} MLColor;

typedef struct MLCanvas {
    uint32_t width;  // Width of the canvas in px
    uint32_t height; // Height of the canvas in px
    MLColor *data;   // Pixel data, as rgba
    float *depth;    // (Optional) Depth buffer
} MLCanvas;

typedef struct MLFont {
    uint32_t glyph_width;
    uint32_t glyph_height;
    const char *glyphs;
} MLFont;

typedef struct Array {
    void *data;
    size_t count;
    size_t capacity;
} Array;

typedef struct OBJ {
    Array vertices;
    Array faces;
} OBJ;

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

// 3D utilities
MLPoint3D MLPoint3DMake(float x, float y, float z);
MLPoint3D MLPoint3DRotateY(MLPoint3D point, MLPoint3D center, double theta);
MLPoint2D MLPoint3DProject(MLPoint3D point, MLCamera cam);
float MLDistScaleAtZ(float dist, float z, MLCamera cam);

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
void MLCanvasFillTriangleInterpolated(MLCanvas *const canvas, MLPoint2D v1,
                                      MLPoint2D v2, MLPoint2D v3);
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

// OBJ utils
OBJ OBJMake();
OBJ OBJLoadFromFile(const char *filename);
size_t OBJVertexCount(const OBJ *const obj);
size_t OBJFaceCount(const OBJ *const obj);
void OBJAddVertex(OBJ *const obj, float x, float y, float z);
void OBJAddFace(OBJ *const obj, size_t i, size_t j, size_t k);
MLPoint3D OBJGetVertex(const OBJ *const obj, size_t i, float scale);
MLVector3Size OBJGetFace(const OBJ *const obj, size_t i);
void OBJFree(OBJ *obj);

// Misc utilities
float lerpf(float t, float a, float b);
double lerpd(double t, double a, double b);

// Arrays
void *ArrayGet(const Array *const arr, size_t i, size_t item_size);
Array ArrayMake(size_t item_size, size_t capacity);
void ArrayResize(Array *const arr, size_t size, size_t item_size);
void ArrayFree(Array *arr);

#define ARRAY_MAKE(type, capacity) ArrayMake(sizeof(type), (capacity))
#define ARRAY_RESIZE(type, arr, size) ArrayResize((arr), (size), sizeof(type))
#define ARRAY_GET(type, arr, i) (*(type *)ArrayGet((arr), (i), sizeof(type)))
#define ARRAY_APPEND(type, arr, item)                                          \
    do {                                                                       \
        if ((arr)->count >= (arr)->capacity) {                                 \
            ARRAY_RESIZE(type, arr, (arr)->capacity * 2);                      \
        }                                                                      \
        ((type *)(arr)->data)[(arr)->count++] = (item);                        \
    } while (0)

#endif // MOLUVI_H
