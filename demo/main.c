#include <stdio.h>
#include <string.h>
#include <math.h>

#include "../include/canvas.h"
#include "../include/renderer.h"
#include "../include/math3d.h"

#define WIDTH 512
#define HEIGHT 512

vec3_t cube_vertices[] = {
    {-1, -1, -1}, 
    {1, -1, -1}, 
    {1, 1, -1}, 
    {-1, 1, -1},
    {-1, -1, 1}, 
    {1, -1, 1}, 
    {1, 1, 1}, 
    {-1, 1, 1}
};

int cube_edges[][2] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0},
    {4, 5}, {5, 6}, {6, 7}, {7, 4},
    {0, 4}, {1, 5}, {2, 6}, {3, 7}
};

int main() {
    canvas_t* canvas = create_canvas(WIDTH, HEIGHT);
    mesh_t cube = {
        .vertices = cube_vertices,
        .vertex_count = 8,
        .edges = cube_edges,
        .edge_count = 12
    };

    for (int frame = 0; frame < 60; frame++) {
        float angle = frame * 0.05f;

        mat4_t roataion = mat4_rotate_xyz(angle, angle, angle);
        mat4_t scale = mat4_scale(0.8f, 0.8f, 0.8f);
        mat4_t model = mat4_mul(roataion, scale);

        //mat4_t model = mat4_rotate_xyz(angle, angle, angle);
        mat4_t view = mat4_translate(0, 0, -5.0f);
        mat4_t projection = mat4_frustum_asymmetric(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 10.0f);

        mat4_t vp = mat4_mul(projection, view);
        mat4_t mvp = mat4_mul(vp, model);

        // Draw
        render_wireframe(canvas, cube, mvp);

        // Save frame as PGM
        char filename[64];
        sprintf(filename, "frame_%03d.pgm", frame);
        FILE* f = fopen(filename, "w");
        fprintf(f, "P2\n%d %d\n255\n", WIDTH, HEIGHT);
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                int val = (int)(fminf(canvas->pixels[y][x], 1.0f) * 255.0f);
                fprintf(f, "%d ", val);
            }
            fprintf(f, "\n");
        }
        fclose(f);

        // Clear canvas for next frame
        for (int y = 0; y < HEIGHT; y++) {
            memset(canvas->pixels[y], 0, WIDTH * sizeof(float));
        }
    }

    destroy_canvas(canvas);
    return 0;
}
