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

struct vec2i {
    int64_t x;
    int64_t y;
};

struct vec3f {
    float x;
    float y;
    float z;
};

struct vec3z {
    size_t x;
    size_t y;
    size_t z;
};

typedef struct vec2i point2_t;
typedef struct vec3f point3_t;

struct camera {
    float focal_len; // The focal length of the camera
    float dist;      // The distance of camera from screen
    uint32_t width;  // The screen width
    uint32_t height; // The screen height
};

struct rgba {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

struct canvas {
    uint32_t width;    // Width of the canvas in px
    uint32_t height;   // Height of the canvas in px
    struct rgba *data; // Pixel data, as rgba
    float *depth;      // (Optional) Depth buffer
};

// TODO: Hide struct canvas
typedef struct canvas canvas_t;

struct font {
    uint32_t glyph_width;
    uint32_t glyph_height;
    const char *glyphs;
};

// TODO: Hide struct font
typedef struct font font_t;

struct arraylist {
    void *data;
    size_t count;
    size_t capacity;
};

// TODO: Hide struct arraylist
typedef struct arraylist arraylist_t;

struct obj {
    arraylist_t vertices;
    arraylist_t faces;
};

// TODO: Hide struct obj
typedef struct obj obj_t;

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
#define COLOR_BLACK (struct rgba){0, 0, 0, 255}
#define COLOR_WHITE (struct rgba){255, 255, 255, 255}
#define COLOR_RED (struct rgba){255, 0, 0, 255}
#define COLOR_GREEN (struct rgba){0, 255, 0, 255}
#define COLOR_BLUE (struct rgba){0, 0, 255, 255}

#ifndef C
#define C(hex) hex_to_rgba(hex)
#else
#define COLOR(hex) hex_to_rgba(hex)
#endif

// Canvas creation & utils
int canvas_init(canvas_t *const canvas, uint32_t width, uint32_t height,
                struct rgba fill);
int canvas_use_depth(canvas_t *const canvas);
int canvas_depth_reset(canvas_t *const canvas);
void canvas_cleanup(canvas_t *const canvas);
int canvas_get_px(const canvas_t *const canvas, uint32_t x, uint32_t y,
                  struct rgba *px);
int canvas_set_px(canvas_t *const canvas, uint32_t x, uint32_t y,
                  struct rgba color);
int canvas_blend_px(canvas_t *const canvas, uint32_t x, uint32_t y,
                    struct rgba color);

// 3D utilities
void point3_rotate(point3_t *point, point3_t center, float theta);
point2_t point3_proj(point3_t point, struct camera cam);
float scale_z(float val, float z, struct camera cam);
int canvas_proj_tri(canvas_t *const canvas, point3_t *const vertices,
                    struct camera cam);

// Color functions
uint32_t rgba_to_hex(struct rgba color);
struct rgba hex_to_rgba(uint32_t hex);
struct rgba rgba_convert_grayscale(struct rgba color);
struct rgba rgba_alpha_blend(struct rgba fg, struct rgba bg);
bool rgba_eql(struct rgba c1, struct rgba c2);
struct rgba rgba_diff_blend(struct rgba c1, struct rgba c2);

// Canvas fills & draws
int canvas_fill(canvas_t *const canvas, struct rgba color);
int canvas_fill_rect(canvas_t *const canvas, uint32_t x, uint32_t y,
                     uint32_t width, uint32_t height, struct rgba color);
int canvas_fill_circle(canvas_t *const canvas, int64_t center_x,
                       int64_t center_y, uint32_t radius, struct rgba color);
int canvas_fill_tri(canvas_t *const canvas, int64_t x0, int64_t y0, int64_t x1,
                    int64_t y1, int64_t x2, int64_t y2, struct rgba color);
void canvas_fill_triInterpolated(canvas_t *const canvas, point2_t v1,
                                 point2_t v2, point2_t v3);
int canvas_fill_quad(canvas_t *const canvas, point2_t p1, point2_t p2,
                     point2_t p3, point2_t p4, struct rgba color);
int canvas_draw_line(canvas_t *const canvas, uint32_t x0, uint32_t y0,
                     uint32_t x1, uint32_t y1, struct rgba color,
                     uint32_t thiccness);

// Text drawing
void canvas_draw_char(canvas_t *const canvas, char c, uint32_t x, uint32_t y,
                      font_t font, uint32_t font_size, struct rgba color);
void canvas_write_string(canvas_t *const canvas, const char *str, uint32_t x,
                         uint32_t y, font_t font, uint32_t font_size,
                         struct rgba color);

// Canvas rendering
void canvas_render_ppm(const canvas_t *const canvas, const char *filename);
canvas_t canvas_load_ppm(const char *filename);

// font_t utils
const char *font_get_glyph(font_t font, char c);

// obj_t utils
int obj_init(obj_t *const obj);
int obj_load(obj_t *const obj, const char *filename);
size_t obj_vertex_count(const obj_t *const obj);
size_t obj_face_count(const obj_t *const obj);
void obj_add_vertex(obj_t *const obj, float x, float y, float z);
void obj_add_face(obj_t *const obj, size_t i, size_t j, size_t k);
point3_t obj_get_vertex(const obj_t *const obj, size_t i, float scale);
struct vec3z obj_get_face(const obj_t *const obj, size_t i);
void obj_cleanup(obj_t *obj);

// Misc utilities
float lerpf(float t, float a, float b);
double lerpd(double t, double a, double b);

// Arrays
void *array_get(const arraylist_t *const arr, size_t i, size_t item_size);
int array_init(arraylist_t *const array, size_t item_size, size_t capacity);
int array_resize(arraylist_t *const arr, size_t size, size_t item_size);
void array_cleanup(arraylist_t *arr);

#define ARRAY_MAKE(arr, type, capacity)                                        \
    array_init(arr, sizeof(type), (capacity))
#define ARRAY_RESIZE(type, arr, size) array_resize((arr), (size), sizeof(type))
#define ARRAY_GET(type, arr, i) (*(type *)array_get((arr), (i), sizeof(type)))
#define ARRAY_APPEND(type, arr, item)                                          \
    do {                                                                       \
        if ((arr)->count >= (arr)->capacity) {                                 \
            int ret = ARRAY_RESIZE(type, arr, (arr)->capacity * 2);            \
        }                                                                      \
        ((type *)(arr)->data)[(arr)->count++] = (item);                        \
    } while (0)

#endif // MOLUVI_H
