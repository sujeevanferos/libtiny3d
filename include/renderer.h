#ifndef RENDERER_H
#define RENDERER_H

#include "canvas.h"
#include "math3d.h"

typedef struct {
    vec3_t* vertices;
    int vertex_count;
    int (*edges)[2];
    int edge_count;
} mesh_t;

vec3_t mat4_mul_vec3(mat4_t m, vec3_t v);
vec3_t project_vertex(vec3_t v, mat4_t mvp, int screen_width, int screen_height);
void render_wireframe(canvas_t* canvas, mesh_t *mesh, mat4_t mvp);
int clip_to_circular_viewport(canvas_t* canvas, float x, float y);

#endif