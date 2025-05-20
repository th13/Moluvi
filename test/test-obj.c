#include "../src/moluvi.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

void OBJPrintVertices(const OBJ *const obj) {
    for (size_t i = 0; i < obj->vertices.count; i += 3) {
        printf("v %f %f %f\n", ARRAY_GET(float, &obj->vertices, i),
               ARRAY_GET(float, &obj->vertices, i + 1),
               ARRAY_GET(float, &obj->vertices, i + 2));
    }
}

void OBJPrintFaces(const OBJ *const obj) {
    for (size_t i = 0; i < obj->faces.count; i += 3) {
        printf("f %zu %zu %zu\n", ARRAY_GET(size_t, &obj->faces, i),
               ARRAY_GET(size_t, &obj->faces, i + 1),
               ARRAY_GET(size_t, &obj->faces, i + 2));
    }
}

int main() {
    OBJ obj = OBJLoadFromFile("vendor/teapot.obj");
    OBJPrintVertices(&obj);
    OBJPrintFaces(&obj);
    OBJFree(&obj);
}
