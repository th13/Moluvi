#include "../src/moluvi.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

void obj_print_vertices(const obj_t *const obj) {
    for (size_t i = 0; i < obj->vertices.count; i += 3) {
        printf("v %f %f %f\n", ARRAY_GET(float, &obj->vertices, i),
               ARRAY_GET(float, &obj->vertices, i + 1),
               ARRAY_GET(float, &obj->vertices, i + 2));
    }
}

void obj_print_faces(const obj_t *const obj) {
    for (size_t i = 0; i < obj->faces.count; i += 3) {
        printf("f %zu %zu %zu\n", ARRAY_GET(size_t, &obj->faces, i),
               ARRAY_GET(size_t, &obj->faces, i + 1),
               ARRAY_GET(size_t, &obj->faces, i + 2));
    }
}

int main() {
    int ret;
    obj_t obj = {0};
    ret = obj_load(&obj, "vendor/teapot.obj");
    if (ret < 0) return ret;
    obj_print_vertices(&obj);
    obj_print_faces(&obj);
    obj_cleanup(&obj);
}
