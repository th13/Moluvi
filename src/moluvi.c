#include "moluvi.h"
#include <float.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

inline MLPoint2D MLPoint2DMake(int64_t x, int64_t y) {
    return (MLPoint2D){x, y};
}

inline MLPoint3D MLPoint3DMake(float x, float y, float z) {
    return (MLPoint3D){x, y, z};
}

MLPoint3D MLPoint3DRotateY(MLPoint3D point, MLPoint3D center, double theta) {
    double sin_theta = sin(theta);
    double cos_theta = cos(theta);

    MLPoint3D offset = {point.x - center.x, point.y - center.y,
                        point.z - center.z};
    MLPoint3D rotated;
    rotated.x = offset.x * cos_theta - offset.z * sin_theta + center.x;
    rotated.y = point.y;
    rotated.z = offset.x * sin_theta + offset.z * cos_theta + center.z;
    return rotated;
}

MLPoint2D MLPoint3DProject(MLPoint3D point, MLCamera cam) {
    float z_depth = point.z + cam.dist;
    return MLPoint2DMake(
        (point.x * cam.focal_len) / z_depth + (float)cam.width / 2.,
        // Invert y as our 2D coordinate system has positive y pointing down
        (-point.y * cam.focal_len) / z_depth + (float)cam.height / 2.);
}

inline float MLDistScaleAtZ(float dist, float z, MLCamera cam) {
    return dist * cam.focal_len / (z + cam.dist);
}

static uint8_t ComponentBlend(uint8_t bg, uint8_t fg, uint8_t a) {
    return (uint8_t)((fg * a + bg * (255 - a)) / 255);
}

uint32_t MLColorToHex(MLColor color) {
    uint32_t hex = color.r;
    hex |= (uint32_t)color.g << 8;
    hex |= (uint32_t)color.b << 16;
    hex |= (uint32_t)color.a << 24;
    return hex;
}

MLColor MLColorFromHex(uint32_t hex) {
    MLColor color = {0};
    color.r = (uint8_t)(hex & 0xFF);
    color.g = (uint8_t)((hex >> 8) & 0xFF);
    color.b = (uint8_t)((hex >> 16) & 0xFF);
    color.a = (uint8_t)((hex >> 24) & 0xFF);
    return color;
}

MLColor MLColorToGrayscale(MLColor color) {
    uint32_t gray = (299 * color.r + 587 * color.g + 114 * color.b) / 1000;
    gray = MIN(255, gray);
    return (MLColor){.r = (uint8_t)gray,
                     .g = (uint8_t)gray,
                     .b = (uint8_t)gray,
                     .a = color.a};
}

MLColor MLColorBlend(MLColor fg, MLColor bg) {
    fg.r = ComponentBlend(bg.r, fg.r, fg.a);
    fg.g = ComponentBlend(bg.g, fg.g, fg.a);
    fg.b = ComponentBlend(bg.b, fg.b, fg.a);
    fg.a = 0xFF;
    return fg;
}

MLColor MLColorDifferenceBlend(MLColor c1, MLColor c2) {
    MLColor blend = {0, 0, 0, 255};
    blend.r = abs(c1.r - c2.r);
    blend.g = abs(c1.g - c2.g);
    blend.b = abs(c1.b - c2.b);
    return blend;
}

bool MLColorEqual(MLColor c1, MLColor c2) {
    return c1.r == c2.r && c1.g == c2.g && c1.b == c2.b && c1.a == c2.a;
}

MLCanvas MLCanvasMake(uint32_t width, uint32_t height, MLColor fill) {
    MLColor *data = malloc(width * height * sizeof(MLColor));
    for (uint32_t i = 0; i < width * height; i++) {
        data[i] = fill;
    }
    return (MLCanvas){.width = width, .height = height, .data = data};
}

void MLCanvasUseDepth(MLCanvas *const canvas) {
    canvas->depth = malloc(canvas->width * canvas->height * sizeof(float));
    canvas_depth_reset(canvas);
}

void canvas_depth_reset(MLCanvas *const canvas) {
    for (int i = 0; i < canvas->width * canvas->height; i++) {
        canvas->depth[i] = FLT_MAX;
    }
}

void MLCanvasDestroy(MLCanvas *canvas) {
    if (!canvas)
        return;
    if (canvas->data)
        free(canvas->data);
    if (canvas->depth)
        free(canvas->depth);
}

MLColor MLCanvasGetPixel(const MLCanvas *const canvas, uint32_t x, uint32_t y) {
    assert(x >= 0 && x < canvas->width && y >= 0 && y < canvas->height);
    return canvas->data[y * canvas->width + x];
}

void MLCanvasSetPixel(MLCanvas *const canvas, uint32_t x, uint32_t y,
                      MLColor color) {
    uint32_t idx = y * canvas->width + x;
    if (canvas->depth) {
        float z = canvas->depth[idx];
    }
    canvas->data[y * canvas->width + x] = color;
}

void MLCanvasBlendPixel(MLCanvas *const canvas, uint32_t x, uint32_t y,
                        MLColor color) {
    MLColor bg = MLCanvasGetPixel(canvas, x, y);
    MLColor blend = MLColorBlend(color, bg);
    MLCanvasSetPixel(canvas, x, y, blend);
}

void MLCanvasFill(MLCanvas *const canvas, MLColor color) {
    for (uint32_t x = 0; x < canvas->width; x++) {
        for (uint32_t y = 0; y < canvas->height; y++) {
            MLCanvasSetPixel(canvas, x, y, color);
        }
    }
}

void MLCanvasFillRect(MLCanvas *const canvas, uint32_t x, uint32_t y,
                      uint32_t width, uint32_t height, MLColor color) {
    for (uint32_t iy = y; iy < height + y; iy++) {
        for (uint32_t ix = x; ix < width + x; ix++) {
            MLCanvasBlendPixel(canvas, ix, iy, color);
        }
    }
}

void MLCanvasFillCircle(MLCanvas *const canvas, int64_t center_x,
                        int64_t center_y, uint32_t radius, MLColor color) {
    uint32_t start_x = MAX(0, center_x - (int64_t)radius);
    uint32_t start_y = MAX(0, center_y - (int64_t)radius);
    uint32_t end_x =
        MIN(canvas->width - 1, (uint32_t)(center_x + (int64_t)radius));
    uint32_t end_y =
        MIN(canvas->height - 1, (uint32_t)(center_y + (int64_t)radius));

    for (uint32_t ix = start_x; ix <= end_x; ix++) {
        for (uint32_t iy = start_y; iy <= end_y; iy++) {
            int64_t offset_x = (int64_t)ix - center_x;
            int64_t offset_y = (int64_t)iy - center_y;
            if (offset_x * offset_x + offset_y * offset_y <= radius * radius) {
                MLCanvasBlendPixel(canvas, ix, iy, color);
            }
        }
    }
}

typedef void (*TriangleCallback)(MLCanvas *const canvas, int64_t x, int64_t y,
                                 float u, float v, float w, void *ctx);

void TriangleCalculate(MLCanvas *const canvas, MLPoint2D v1, MLPoint2D v2,
                       MLPoint2D v3, TriangleCallback callback, void *ctx) {
    int64_t x0 = v1.x, y0 = v1.y;
    int64_t x1 = v2.x, y1 = v2.y;
    int64_t x2 = v3.x, y2 = v3.y;

    // Bounding region
    int64_t start_x = MIN(MIN(x0, x1), x2);
    int64_t start_y = MIN(MIN(y0, y1), y2);
    int64_t end_x = MAX(MAX(x0, x1), x2);
    int64_t end_y = MAX(MAX(y0, y1), y2);
    if (!(start_x >= 0 && start_y >= 0 && end_x < canvas->width &&
          end_y < canvas->height)) {
        return;
    }

    int64_t dy01 = y0 - y1;
    int64_t dy12 = y1 - y2;
    int64_t dy20 = y2 - y0;
    int64_t area = x0 * dy12 + x1 * dy20 + x2 * dy01;
    if (area == 0)
        return; // no degenerate triangles

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
                callback(canvas, ix, iy, u, v, w, ctx);
            }
        }
    }
}

static void TriangleFillAtPoint(MLCanvas *const canvas, int64_t x, int64_t y,
                                float u, float v, float w, void *ctx) {
    MLCanvasBlendPixel(canvas, x, y, *(MLColor *)ctx);
}

void MLCanvasFillTriangle(MLCanvas *const canvas, int64_t x0, int64_t y0,
                          int64_t x1, int64_t y1, int64_t x2, int64_t y2,
                          MLColor color) {
    MLPoint2D v1 = {x0, y0};
    MLPoint2D v2 = {x1, y1};
    MLPoint2D v3 = {x2, y2};
    TriangleCalculate(canvas, v1, v2, v3, &TriangleFillAtPoint, &color);
}

static MLColor color_lerp_rgb(float u, float v, float w) {

    return (MLColor){
        (uint8_t)(u * 255.),
        (uint8_t)(v * 255.),
        (uint8_t)(w * 255.),
        255,
    };
}

static void TriangleInterpolateRGBAtPoint(MLCanvas *const canvas, int64_t x,
                                          int64_t y, float u, float v, float w,
                                          void *ctx) {
    MLCanvasBlendPixel(canvas, x, y, color_lerp_rgb(u, v, w));
}

void MLCanvasFillTriangleInterpolated(MLCanvas *const canvas, MLPoint2D v1,
                                      MLPoint2D v2, MLPoint2D v3) {
    TriangleCalculate(canvas, v1, v2, v3, &TriangleInterpolateRGBAtPoint, NULL);
}

void MLCanvasFillQuad(MLCanvas *const canvas, MLPoint2D p1, MLPoint2D p2,
                      MLPoint2D p3, MLPoint2D p4, MLColor color) {
    // We require the quad to be sorted for now.
    assert(p1.x < p2.x && p1.x < p3.x && p1.x < p4.x);
    assert(p4.x > p3.x && p4.x > p2.x && p4.x > p1.x);
    MLCanvasFillTriangle(canvas, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, color);
    MLCanvasFillTriangle(canvas, p4.x, p4.y, p2.x, p2.y, p3.x, p3.y, color);
}

/**
 * Draws a line from point (x0, y0) to (x1, y1) of width 1.
 *
 * @todo Antialiasing
 */
void MLCanvasDrawLine(MLCanvas *const canvas, uint32_t x0, uint32_t y0,
                      uint32_t x1, uint32_t y1, MLColor color,
                      uint32_t thiccness) {
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
                    MLCanvasBlendPixel(canvas, x, iy, color);
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
                    MLCanvasBlendPixel(canvas, x_thicc, iy, color);
                }
            }
        }

        for (uint32_t y_thicc = ry0; y_thicc < ry0 + thiccness; y_thicc++) {
            if (y_thicc < canvas->height && ix < canvas->width) {
                MLCanvasBlendPixel(canvas, ix, y_thicc, color);
            }
        }
    }
}

// 3D

void TriangleInterpolateWithZ(MLCanvas *const canvas, int64_t x, int64_t y,
                              float u, float v, float w, void *ctx) {
    MLPoint3D *vertices = (MLPoint3D *)ctx;
    MLColor color = color_lerp_rgb(u, v, w);
    float z = vertices[0].z * u + vertices[1].z * v + vertices[2].z * w;

    if (z >= canvas->depth[y * canvas->width + x])
        return;

    MLCanvasBlendPixel(canvas, x, y, color);
    canvas->depth[y * canvas->width + x] = z;
}

void MLCanvasProjectTriangle(MLCanvas *const canvas, MLPoint3D *vertices,
                             MLCamera cam) {

    MLPoint2D proj[3] = {
        MLPoint3DProject(vertices[0], cam),
        MLPoint3DProject(vertices[1], cam),
        MLPoint3DProject(vertices[2], cam),
    };

    TriangleCalculate(canvas, proj[0], proj[1], proj[2],
                      &TriangleInterpolateWithZ, vertices);
}

/* Text */

const char *MLFontGetGlyph(MLFont font, char c) {
    return &font.glyphs[c * sizeof(char) * font.glyph_width *
                        font.glyph_height];
}

/**
 * Draws a character at point (x, y).
 */
void MLCanvasDrawChar(MLCanvas *const canvas, char c, uint32_t x, uint32_t y,
                      MLFont font, uint32_t font_size, MLColor color) {
    const char *glyph = MLFontGetGlyph(font, c);
    uint32_t end_x = x + font_size * font.glyph_width - 1;
    uint32_t end_y = y + font_size * font.glyph_height - 1;
    assert(end_x < canvas->width && end_y < canvas->height);

    for (uint32_t ix = x; ix <= end_x; ix++) {
        for (uint32_t iy = y; iy <= end_y; iy++) {
            uint32_t i_glyph = ((iy - y) / font_size) * font.glyph_width +
                               ((ix - x) / font_size);
            if (glyph[i_glyph] == 1) {
                MLCanvasBlendPixel(canvas, ix, iy, color);
            }
        }
    }
}

/**
 * Writes text to the canvas starting at point (x, y).
 */
void MLCanvasWriteString(MLCanvas *const canvas, const char *str, uint32_t x,
                         uint32_t y, MLFont font, uint32_t font_size,
                         MLColor color) {
    uint32_t len = strlen(str);
    if (len == 0)
        return;
    assert(x + len * font.glyph_width < canvas->width &&
           y + font.glyph_height < canvas->height);

    for (uint32_t i = 0; i < len; i++) {
        char c = str[i];
        uint32_t ix = x + i * font_size * font.glyph_width;
        MLCanvasDrawChar(canvas, c, ix, y, font, font_size, color);
    }
}

/**
 * Renders a canvas to a file in PPM P6 format.
 */
void MLCanvasRenderPPM(const MLCanvas *const canvas, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Could not open file");
        return;
    }

    fprintf(file, "P6\n%u %u 255\n", canvas->width, canvas->height);
    for (uint32_t i = 0; i < canvas->width * canvas->height; i++) {
        MLColor color = canvas->data[i];
        fwrite(&color.r, sizeof(uint8_t), 1, file);
        fwrite(&color.g, sizeof(uint8_t), 1, file);
        fwrite(&color.b, sizeof(uint8_t), 1, file);
    }
    fclose(file);
}

/**
 * Loads a canvas from PPM P6 file.
 */
MLCanvas MLCanvasLoadPPM(const char *filename) {
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
        perror("MLCanvasLoadPPM currently only supports P6 format\n");
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

    // Interpret bytes as colors (MLColor)
    MLColor *data = malloc(sizeof(MLColor) * width * height);
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
        MLColor color = {0};
        color.r = raw_data[i * 3 + 0];
        color.g = raw_data[i * 3 + 1];
        color.b = raw_data[i * 3 + 2];
        color.a = 0xFF;
        data[i] = color;
    }

    fclose(file);

    return (MLCanvas){.width = width, .height = height, .data = data};
}

OBJ OBJMake() {
    return (OBJ){.vertices = ARRAY_MAKE(float, 256),
                 .faces = ARRAY_MAKE(size_t, 512)};
}

OBJ OBJLoadFromFile(const char *filename) {
    FILE *obj_file = fopen(filename, "r");
    if (!obj_file) {
        fprintf(stderr, "Could not open vendor/teapot.obj");
        exit(1);
    }

    OBJ obj = OBJMake();

    char type;
    float x, y, z;
    while (fscanf(obj_file, "%c %f %f %f\n", &type, &x, &y, &z) == 4) {
        printf("type = %c, %f %f %f\n", type, x, y, z);
        if (type == 'v') {
            OBJAddVertex(&obj, x, y, z);
        } else if (type == 'f') {
            OBJAddFace(&obj, (size_t)x, (size_t)y, (size_t)z);
        } else {
            fprintf(stderr, "WARNING: Unsupported OBJ type '%c' found\n", type);
        }
    }

    return obj;
}

inline size_t OBJVertexCount(const OBJ *const obj) {
    return obj->vertices.count / 3;
}

inline size_t OBJFaceCount(const OBJ *const obj) {
    return obj->faces.count / 3;
}

void OBJAddVertex(OBJ *const obj, float x, float y, float z) {
    ARRAY_APPEND(float, &obj->vertices, x);
    ARRAY_APPEND(float, &obj->vertices, y);
    ARRAY_APPEND(float, &obj->vertices, z);
}

void OBJAddFace(OBJ *const obj, size_t i, size_t j, size_t k) {
    // OBJ stores vertex indices 1-indexed, we'll correct that here
    ARRAY_APPEND(size_t, &obj->faces, i - 1);
    ARRAY_APPEND(size_t, &obj->faces, j - 1);
    ARRAY_APPEND(size_t, &obj->faces, k - 1);
}

MLPoint3D OBJGetVertex(const OBJ *const obj, size_t i, float scale) {
    size_t pos = i * 3;
    return (MLPoint3D){ARRAY_GET(float, &obj->vertices, pos) * scale,
                       ARRAY_GET(float, &obj->vertices, pos + 1) * scale,
                       ARRAY_GET(float, &obj->vertices, pos + 2) * scale};
}

MLVector3Size OBJGetFace(const OBJ *const obj, size_t i) {
    size_t pos = i * 3;
    return (MLVector3Size){ARRAY_GET(size_t, &obj->faces, pos),
                           ARRAY_GET(size_t, &obj->faces, pos + 1),
                           ARRAY_GET(size_t, &obj->faces, pos + 2)};
}

void OBJFree(OBJ *obj) {
    ArrayFree(&obj->vertices);
    ArrayFree(&obj->faces);
}

void *ArrayGet(const Array *const arr, size_t i, size_t item_size) {
    if (i >= arr->count) {
        fprintf(stderr, "ERROR: Index %zu out of bounds, count: %zu\n", i,
                arr->count);
        exit(1);
    }
    return arr->data + i * item_size;
}

Array ArrayMake(size_t item_size, size_t capacity) {
    return (Array){
        .data = malloc((item_size) * (capacity)),
        .count = 0,
        .capacity = (capacity),
    };
}

void ArrayFree(Array *arr) {
    if (!arr)
        return;
    free(arr->data);
}

void ArrayResize(Array *const arr, size_t size, size_t item_size) {
    if (arr->count > size) {
        fprintf(stderr, "Cannot resize array smaller than item count");
    }

    arr->capacity = size;
    arr->data = realloc(arr->data, arr->capacity * item_size);
    if (arr->data == NULL) {
        fprintf(stderr, "Failed to realloc array while resizing");
        exit(1);
    }
}

#define LERP(t, a, b) (a) + (t) * ((b) - (a))
inline float lerpf(float t, float a, float b) { return LERP(t, a, b); }
inline double lerpd(double t, double a, double b) { return LERP(t, a, b); }
