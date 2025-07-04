#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../include/canvas.h"
#include "../include/renderer.h"
#include "../include/math3d.h"

#define WIDTH 512
#define HEIGHT 512
#define FRAME_COUNT 60

// Simple .obj-like mesh loader
mesh_t generate_soccer_ball(const char* filename) {
    static vec3_t vertices[1024];
    static int edges[2048][2];
    int v_count = 0, e_count = 0;

    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open soccer ball file");
        exit(1);
    }

    char line[128];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v') {
            float x, y, z;
            sscanf(line, "v %f %f %f", &x, &y, &z);
            vertices[v_count++] = vec3_from_cartesian(x, y, z);
        } else if (line[0] == 'e') {
            int a, b;
            sscanf(line, "e %d %d", &a, &b);
            edges[e_count][0] = a;
            edges[e_count][1] = b;
            e_count++;
        }
    }

    fclose(file);

    mesh_t mesh = {
        .vertices = vertices,
        .vertex_count = v_count,
        .edges = edges,
        .edge_count = e_count
    };
    return mesh;
}

int main() {
    canvas_t* canvas = create_canvas(WIDTH, HEIGHT);

    mesh_t soccer_ball = generate_soccer_ball("soccer_ball.txt");

    for (int frame = 0; frame < FRAME_COUNT; frame++) {
        float angle = frame * 0.05f;

        mat4_t model = mat4_rotate_xyz(angle, angle, angle);
        mat4_t view = mat4_translate(0, 0, -5.0f);
        mat4_t projection = mat4_frustum_asymmetric(-1.5f, 1.5f, -1.5f, 1.5f, 1.5f, 20.0f);

        mat4_t vp = mat4_mul(projection, view);
        mat4_t mvp = mat4_mul(vp, model);

        render_wireframe(canvas, soccer_ball, mvp);

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
