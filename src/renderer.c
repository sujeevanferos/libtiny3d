#include <stdlib.h>

#include "../include/renderer.h"

vec3_t project_vertex(vec3_t v, mat4_t transform, int width, int height) {
    vec3_t p = mat4_mul_vec3(transform, v);
    p.x = (p.x + 1.0f) * 0.5f * width;
    p.y = (1.0f - p.y) * 0.5f * height;
    return p;
}

int clip_to_circular_viewport(canvas_t *canvas, float x, float y) {
    float cx = canvas->width / 2.0f;
    float cy = canvas->height / 2.0f;
    float dx = x - cx;
    float dy = y - cy;
    float r = canvas->width / 2.0f;
    return (dx*dx + dy*dy <= r*r);
}

void render_wireframe(canvas_t *canvas, mesh_t *mesh, mat4_t transform) {
    for (int i = 0; i < mesh->edge_count; i++) {
        vec3_t p0 = project_vertex(mesh->vertices[mesh->edges[i][0]], transform, canvas->width, canvas->height);
        vec3_t p1 = project_vertex(mesh->vertices[mesh->edges[i][1]], transform, canvas->width, canvas->height);
        if (clip_to_circular_viewport(canvas, p0.x, p0.y) && clip_to_circular_viewport(canvas, p1.x, p1.y)) {
            draw_line_f(canvas, p0.x, p0.y, p1.x, p1.y, 1.0f);
        }
    }
}
