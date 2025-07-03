#include <stdio.h>
#include <math.h>
#include "../include/renderer.h"

vec3_t project_vertex(vec3_t v, mat4_t mvp, int screen_width, int screen_height){
    vec3_t transformed = mat4_mul_vec3(mvp, v);

    float sx = (transformed.x + 1.0f) * 0.5f * screen_width;
    float sy = (1.0f - transformed.y) * 0.5f * screen_height;

    printf("Projected: (%.2f, %.2f, %.2f) → Screen: (%.2f, %.2f)\n", 
        transformed.x, transformed.y, transformed.z, sx, sy);


    return vec3_from_cartesian(sx, sy, transformed.z);
}

int clip_to_circular_viewport(canvas_t* canvas, float x, float y){
    float cx = canvas->width /2.0f;
    float cy = canvas->height /2.0f;
    float dx = x - cx;
    float dy = y - cy;
    float r = fminf(canvas->width, canvas->height) / 2.0f;
    return (dx * dx + dy * dy <= r*r);
}

void render_wireframe(canvas_t* canvas, mesh_t mesh, mat4_t mvp){
    for(int i = 0; i < mesh.edge_count; i++){
        vec3_t v0 = project_vertex(mesh.vertices[mesh.edges[i][0]], mvp, canvas->width, canvas->height);
        vec3_t v1 = project_vertex(mesh.vertices[mesh.edges[i][1]], mvp, canvas->width, canvas->height);

        if (clip_to_circular_viewport(canvas, v0.x, v0.y) || 
        clip_to_circular_viewport(canvas, v1.x, v1.y)) {
            draw_line_f(canvas, v0.x, v0.y, v1.x, v1.y, 1.0f);
        } 
   }
}