#include "moluvi.h"
#include <errno.h>
#include <float.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

void point3_rotate(point3_t *point, point3_t center, float theta) {
    double sin_theta = sinf(theta);
    double cos_theta = cosf(theta);

    point3_t offset = {point->x - center.x, point->y - center.y,
                       point->z - center.z};
    point->x = offset.x * cos_theta - offset.z * sin_theta + center.x;
    point->z = offset.x * sin_theta + offset.z * cos_theta + center.z;
}

point2_t point3_proj(point3_t point, struct camera cam) {
    float z_depth = point.z + cam.dist;
    return (point2_t){
        (point.x * cam.focal_len) / z_depth + (float)cam.width / 2.,
        // Invert y as our 2D coordinate system has positive y
        // pointing down
        (-point.y * cam.focal_len) / z_depth + (float)cam.height / 2.};
}

inline float scale_z(float val, float z, struct camera cam) {
    return val * cam.focal_len / (z + cam.dist);
}

static uint8_t ComponentBlend(uint8_t bg, uint8_t fg, uint8_t a) {
    return (uint8_t)((fg * a + bg * (255 - a)) / 255);
}

uint32_t rgba_to_hex(struct rgba color) {
    uint32_t hex = color.r;
    hex |= (uint32_t)color.g << 8;
    hex |= (uint32_t)color.b << 16;
    hex |= (uint32_t)color.a << 24;
    return hex;
}

struct rgba hex_to_rgba(uint32_t hex) {
    struct rgba color = {0};
    color.r = (uint8_t)(hex & 0xFF);
    color.g = (uint8_t)((hex >> 8) & 0xFF);
    color.b = (uint8_t)((hex >> 16) & 0xFF);
    color.a = (uint8_t)((hex >> 24) & 0xFF);
    return color;
}

struct rgba rgba_convert_grayscale(struct rgba color) {
    uint32_t gray = (299 * color.r + 587 * color.g + 114 * color.b) / 1000;
    gray = MIN(255, gray);
    return (struct rgba){.r = (uint8_t)gray,
                         .g = (uint8_t)gray,
                         .b = (uint8_t)gray,
                         .a = color.a};
}

struct rgba rgba_alpha_blend(struct rgba fg, struct rgba bg) {
    fg.r = ComponentBlend(bg.r, fg.r, fg.a);
    fg.g = ComponentBlend(bg.g, fg.g, fg.a);
    fg.b = ComponentBlend(bg.b, fg.b, fg.a);
    fg.a = 0xFF;
    return fg;
}

struct rgba rgba_diff_blend(struct rgba c1, struct rgba c2) {
    struct rgba blend = {0, 0, 0, 255};
    blend.r = abs(c1.r - c2.r);
    blend.g = abs(c1.g - c2.g);
    blend.b = abs(c1.b - c2.b);
    return blend;
}

bool rgba_eql(struct rgba c1, struct rgba c2) {
    return c1.r == c2.r && c1.g == c2.g && c1.b == c2.b && c1.a == c2.a;
}

int canvas_init(canvas_t *const canvas, uint32_t width, uint32_t height,
                struct rgba fill) {
    struct rgba *data = malloc(width * height * sizeof(struct rgba));
    if (!data)
        return -ENOMEM;

    for (uint32_t i = 0; i < width * height; i++) {
        data[i] = fill;
    }

    canvas->width = width;
    canvas->height = height;
    canvas->data = data;
    canvas->depth = NULL;
    return 0;
}

int canvas_use_depth(canvas_t *const canvas) {
    canvas->depth = malloc(canvas->width * canvas->height * sizeof(float));
    if (!canvas->depth)
        return -ENOMEM;
    canvas_depth_reset(canvas);
    return 0;
}

int canvas_depth_reset(canvas_t *const canvas) {
    if (canvas->depth == NULL)
        return -EINVAL;
    for (uint32_t i = 0; i < canvas->width * canvas->height; i++) {
        canvas->depth[i] = FLT_MAX;
    }
    return 0;
}

void canvas_cleanup(canvas_t *const canvas) {
    if (!canvas)
        return;

    if (canvas->data) {
        free(canvas->data);
        canvas->data = NULL;
    }

    if (canvas->depth) {
        free(canvas->depth);
        canvas->depth = NULL;
    }
}

static inline bool canvas_point_in_range(const canvas_t *const canvas,
                                         uint32_t x, uint32_t y) {
    return x >= 0 && x < canvas->width && y >= 0 && y < canvas->height;
}

static inline bool canvas_valid(const canvas_t *const canvas) {
    return canvas != NULL && canvas->data != NULL;
}

int canvas_get_px(const canvas_t *const canvas, uint32_t x, uint32_t y,
                  struct rgba *px) {
    if (!canvas_point_in_range(canvas, x, y))
        return -EDOM;
    *px = canvas->data[y * canvas->width + x];
    return 0;
}

int canvas_set_px(canvas_t *const canvas, uint32_t x, uint32_t y,
                  struct rgba color) {
    if (!canvas_point_in_range(canvas, x, y)) {
        fprintf(stderr, "Got asked to set px out of range (%d, %d)\n", x, y);
        return -EDOM;
    }
    canvas->data[y * canvas->width + x] = color;
    return 0;
}

int canvas_blend_px(canvas_t *const canvas, uint32_t x, uint32_t y,
                    struct rgba color) {
    int ret;
    struct rgba bg;
    ret = canvas_get_px(canvas, x, y, &bg);
    if (ret < 0)
        return ret;

    struct rgba blend = rgba_alpha_blend(color, bg);
    return canvas_set_px(canvas, x, y, blend);
}

int canvas_fill(canvas_t *const canvas, struct rgba color) {
    if (!canvas_valid(canvas))
        return -EINVAL;

    int ret;
    for (uint32_t x = 0; x < canvas->width; x++) {
        for (uint32_t y = 0; y < canvas->height; y++) {
            ret = canvas_set_px(canvas, x, y, color);
            if (ret < 0)
                return ret;
        }
    }
    return 0;
}

int canvas_fill_rect(canvas_t *const canvas, uint32_t x, uint32_t y,
                     uint32_t width, uint32_t height, struct rgba color) {
    if (!canvas_valid(canvas))
        return -EINVAL;

    int ret;
    uint32_t clip_x = MIN(x + width, canvas->width);
    uint32_t clip_y = MIN(y + height, canvas->height);

    for (uint32_t iy = y; iy < clip_y; iy++) {
        for (uint32_t ix = x; ix < clip_x; ix++) {
            ret = canvas_blend_px(canvas, ix, iy, color);
            if (ret < 0)
                return ret;
        }
    }
    return 0;
}

int canvas_fill_circle(canvas_t *const canvas, int64_t center_x,
                       int64_t center_y, uint32_t radius, struct rgba color) {
    int ret;
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
                ret = canvas_blend_px(canvas, ix, iy, color);
                if (ret < 0)
                    return ret;
            }
        }
    }

    return 0;
}

typedef void (*barycentric_callback_t)(canvas_t *const canvas, int64_t x,
                                       int64_t y, float u, float v, float w,
                                       void *ctx);

int calc_tri_barycentric(canvas_t *const canvas, point2_t v1, point2_t v2,
                         point2_t v3, barycentric_callback_t callback,
                         void *ctx) {
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
        return -EDOM;
    }

    int64_t dy01 = y0 - y1;
    int64_t dy12 = y1 - y2;
    int64_t dy20 = y2 - y0;
    int64_t area = x0 * dy12 + x1 * dy20 + x2 * dy01;
    if (area == 0)
        return -EDOM; // no degenerate triangles

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

    return 0;
}

static void tri_fill_at_point(canvas_t *const canvas, int64_t x, int64_t y,
                                float u, float v, float w, void *ctx) {
    canvas_blend_px(canvas, x, y, *(struct rgba *)ctx);
}

int canvas_fill_tri(canvas_t *const canvas, int64_t x0, int64_t y0, int64_t x1,
                    int64_t y1, int64_t x2, int64_t y2, struct rgba color) {
    point2_t v1 = {x0, y0};
    point2_t v2 = {x1, y1};
    point2_t v3 = {x2, y2};
    return calc_tri_barycentric(canvas, v1, v2, v3, &tri_fill_at_point,
                                &color);
}

static struct rgba color_lerp_rgb(float u, float v, float w) {

    return (struct rgba){
        (uint8_t)(u * 255.),
        (uint8_t)(v * 255.),
        (uint8_t)(w * 255.),
        255,
    };
}

static void tri_interp_rgb(canvas_t *const canvas, int64_t x, int64_t y,
                           float u, float v, float w, void *ctx) {
    canvas_blend_px(canvas, x, y, color_lerp_rgb(u, v, w));
}

void canvas_fill_triInterpolated(canvas_t *const canvas, point2_t v1,
                                 point2_t v2, point2_t v3) {
    calc_tri_barycentric(canvas, v1, v2, v3, &tri_interp_rgb, NULL);
}

int canvas_fill_quad(canvas_t *const canvas, point2_t p1, point2_t p2,
                     point2_t p3, point2_t p4, struct rgba color) {
    // We require the quad to be sorted for now.
    assert(p1.x < p2.x && p1.x < p3.x && p1.x < p4.x);
    assert(p4.x > p3.x && p4.x > p2.x && p4.x > p1.x);

    int ret;
    ret = canvas_fill_tri(canvas, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, color);
    if (ret < 0)
        return ret;
    ret = canvas_fill_tri(canvas, p4.x, p4.y, p2.x, p2.y, p3.x, p3.y, color);
    if (ret < 0)
        return ret;
    return 0;
}

/**
 * Draws a line from point (x0, y0) to (x1, y1) of width 1.
 *
 * @todo Antialiasing
 */
int canvas_draw_line(canvas_t *const canvas, uint32_t x0, uint32_t y0,
                     uint32_t x1, uint32_t y1, struct rgba color,
                     uint32_t thiccness) {
    int ret;
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
                    ret = canvas_blend_px(canvas, x, iy, color);
                    if (ret < 0)
                        return ret;
                }
            }
        }

        return 0;
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
                    ret = canvas_blend_px(canvas, x_thicc, iy, color);
                    if (ret < 0)
                        return ret;
                }
            }
        }

        for (uint32_t y_thicc = ry0; y_thicc < ry0 + thiccness; y_thicc++) {
            if (y_thicc < canvas->height && ix < canvas->width) {
                ret = canvas_blend_px(canvas, ix, y_thicc, color);
                if (ret < 0)
                    return ret;
            }
        }
    }

    return 0;
}

// 3D

void tri_interp_rgb_depth(canvas_t *const canvas, int64_t x, int64_t y, float u,
                          float v, float w, void *ctx) {
    point3_t *vertices = (point3_t *)ctx;
    struct rgba color = color_lerp_rgb(u, v, w);
    float z = vertices[0].z * u + vertices[1].z * v + vertices[2].z * w;

    if (z >= canvas->depth[y * canvas->width + x])
        return;

    canvas_blend_px(canvas, x, y, color);
    canvas->depth[y * canvas->width + x] = z;
}

int canvas_proj_tri(canvas_t *const canvas, point3_t *const vertices,
                    struct camera cam) {

    point2_t proj[3] = {
        point3_proj(vertices[0], cam),
        point3_proj(vertices[1], cam),
        point3_proj(vertices[2], cam),
    };

    for (int i = 0; i < 3; i++) {
        if (!canvas_point_in_range(canvas, proj[i].x, proj[i].y))
            return -EINVAL;
    }

    calc_tri_barycentric(canvas, proj[0], proj[1], proj[2],
                         &tri_interp_rgb_depth, vertices);
}

/* Text */

const char *font_get_glyph(font_t font, char c) {
    return &font.glyphs[c * sizeof(char) * font.glyph_width *
                        font.glyph_height];
}

/**
 * Draws a character at point (x, y).
 */
void canvas_draw_char(canvas_t *const canvas, char c, uint32_t x, uint32_t y,
                      font_t font, uint32_t font_size, struct rgba color) {
    const char *glyph = font_get_glyph(font, c);
    uint32_t end_x = x + font_size * font.glyph_width - 1;
    uint32_t end_y = y + font_size * font.glyph_height - 1;
    assert(end_x < canvas->width && end_y < canvas->height);

    for (uint32_t ix = x; ix <= end_x; ix++) {
        for (uint32_t iy = y; iy <= end_y; iy++) {
            uint32_t i_glyph = ((iy - y) / font_size) * font.glyph_width +
                               ((ix - x) / font_size);
            if (glyph[i_glyph] == 1) {
                canvas_blend_px(canvas, ix, iy, color);
            }
        }
    }
}

/**
 * Writes text to the canvas starting at point (x, y).
 */
void canvas_write_string(canvas_t *const canvas, const char *str, uint32_t x,
                         uint32_t y, font_t font, uint32_t font_size,
                         struct rgba color) {
    uint32_t len = strlen(str);
    if (len == 0)
        return;
    assert(x + len * font.glyph_width < canvas->width &&
           y + font.glyph_height < canvas->height);

    for (uint32_t i = 0; i < len; i++) {
        char c = str[i];
        uint32_t ix = x + i * font_size * font.glyph_width;
        canvas_draw_char(canvas, c, ix, y, font, font_size, color);
    }
}

/**
 * Renders a canvas to a file in PPM P6 format.
 */
void canvas_render_ppm(const canvas_t *const canvas, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Could not open file");
        return;
    }

    fprintf(file, "P6\n%u %u 255\n", canvas->width, canvas->height);
    for (uint32_t i = 0; i < canvas->width * canvas->height; i++) {
        struct rgba color = canvas->data[i];
        fwrite(&color.r, sizeof(uint8_t), 1, file);
        fwrite(&color.g, sizeof(uint8_t), 1, file);
        fwrite(&color.b, sizeof(uint8_t), 1, file);
    }
    fclose(file);
}

/**
 * Loads a canvas from PPM P6 file.
 */
canvas_t canvas_load_ppm(const char *filename) {
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

    // Interpret bytes as colors (struct rgba)
    struct rgba *data = malloc(sizeof(struct rgba) * width * height);
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
        struct rgba color = {0};
        color.r = raw_data[i * 3 + 0];
        color.g = raw_data[i * 3 + 1];
        color.b = raw_data[i * 3 + 2];
        color.a = 0xFF;
        data[i] = color;
    }

    fclose(file);

    return (canvas_t){.width = width, .height = height, .data = data};
}

int obj_init(obj_t *const obj) {
    int ret;
    ret = ARRAY_MAKE(&obj->vertices, float, 256);
    if (ret < 0)
        return ret;
    ret = ARRAY_MAKE(&obj->faces, size_t, 512);
    if (ret < 0)
        return ret;
    return 0;
}

int obj_load(obj_t *const obj, const char *filename) {
    int ret;
    FILE *obj_file = fopen(filename, "r");
    if (!obj_file) {
        fprintf(stderr, "Could not open vendor/teapot.obj");
        exit(1);
    }

    ret = obj_init(obj);
    if (ret < 0)
        return ret;

    char type;
    float x, y, z;
    while (fscanf(obj_file, "%c %f %f %f\n", &type, &x, &y, &z) == 4) {
        // printf("type = %c, %f %f %f\n", type, x, y, z);
        if (type == 'v') {
            obj_add_vertex(obj, x, y, z);
        } else if (type == 'f') {
            obj_add_face(obj, (size_t)x, (size_t)y, (size_t)z);
        } else {
            fprintf(stderr, "WARNING: Unsupported obj_t type '%c' found\n",
                    type);
        }
    }

    return 0;
}

inline size_t obj_vertex_count(const obj_t *const obj) {
    return obj->vertices.count / 3;
}

inline size_t obj_face_count(const obj_t *const obj) {
    return obj->faces.count / 3;
}

void obj_add_vertex(obj_t *const obj, float x, float y, float z) {
    ARRAY_APPEND(float, &obj->vertices, x);
    ARRAY_APPEND(float, &obj->vertices, y);
    ARRAY_APPEND(float, &obj->vertices, z);
}

void obj_add_face(obj_t *const obj, size_t i, size_t j, size_t k) {
    // obj_t stores vertex indices 1-indexed, we'll correct that here
    ARRAY_APPEND(size_t, &obj->faces, i - 1);
    ARRAY_APPEND(size_t, &obj->faces, j - 1);
    ARRAY_APPEND(size_t, &obj->faces, k - 1);
}

point3_t obj_get_vertex(const obj_t *const obj, size_t i, float scale) {
    size_t pos = i * 3;
    return (point3_t){ARRAY_GET(float, &obj->vertices, pos) * scale,
                      ARRAY_GET(float, &obj->vertices, pos + 1) * scale,
                      ARRAY_GET(float, &obj->vertices, pos + 2) * scale};
}

struct vec3z obj_get_face(const obj_t *const obj, size_t i) {
    size_t pos = i * 3;
    return (struct vec3z){ARRAY_GET(size_t, &obj->faces, pos),
                          ARRAY_GET(size_t, &obj->faces, pos + 1),
                          ARRAY_GET(size_t, &obj->faces, pos + 2)};
}

void obj_cleanup(obj_t *obj) {
    array_cleanup(&obj->vertices);
    array_cleanup(&obj->faces);
}

void *array_get(const arraylist_t *const arr, size_t i, size_t item_size) {
    if (i >= arr->count) {
        fprintf(stderr, "ERROR: Index %zu out of bounds, count: %zu\n", i,
                arr->count);
        exit(1);
    }
    return arr->data + i * item_size;
}

int array_init(arraylist_t *const array, size_t item_size, size_t capacity) {
    *array = (arraylist_t){
        .data = malloc((item_size) * (capacity)),
        .count = 0,
        .capacity = (capacity),
    };

    if (array->data == NULL) {
        return -ENOMEM;
    }

    return 0;
}

void array_cleanup(arraylist_t *arr) {
    if (arr == NULL || arr->data == NULL)
        return;
    free(arr->data);
}

int array_resize(arraylist_t *const arr, size_t size, size_t item_size) {
    if (arr->count > size) {
        fprintf(stderr, "Cannot resize array smaller than item count");
        return -EINVAL;
    }

    arr->capacity = size;
    arr->data = realloc(arr->data, arr->capacity * item_size);
    if (arr->data == NULL) {
        fprintf(stderr, "Failed to realloc array while resizing");
        return -ENOMEM;
    }

    return 0;
}

#define LERP(t, a, b) (a) + (t) * ((b) - (a))
inline float lerpf(float t, float a, float b) { return LERP(t, a, b); }
inline double lerpd(double t, double a, double b) { return LERP(t, a, b); }
