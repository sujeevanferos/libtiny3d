#include <stdio.h>
#include <math.h>

#include "../include/canvas.h"
#include "../include/math3d.h"

#define WIDTH 400
#define HEIGHT 400

int main() {
    canvas_t* canvas = create_canvas(WIDTH, HEIGHT);

    // Define cube vertices (unit cube at origin)
    vec3_t cube[8] = {
        vec3_from_cartesian(-1, -1, -1),
        vec3_from_cartesian( 1, -1, -1),
        vec3_from_cartesian( 1,  1, -1),
        vec3_from_cartesian(-1,  1, -1),
        vec3_from_cartesian(-1, -1,  1),
        vec3_from_cartesian( 1, -1,  1),
        vec3_from_cartesian( 1,  1,  1),
        vec3_from_cartesian(-1,  1,  1)
    };

    // Create transformation matrix: scale → rotate → translate
    mat4_t model = mat4_mul(
        mat4_translate(0.0f, 0.0f, 0.0f),
        mat4_mul(
            mat4_rotate_xyz(0.4f, 0.8f, 0.2f),
            mat4_scale(0.5f, 0.5f, 0.5f)
        )
    );

    // Project and store 2D points
    float projected_x[8], projected_y[8];

    for (int i = 0; i < 8; i++) {
        vec3_t p = mat4_mul_vec3(model, cube[i]);

        // Simple orthographic projection to canvas space
        projected_x[i] = (p.x + 1.0f) * 0.5f * WIDTH;
        projected_y[i] = (1.0f - p.y) * 0.5f * HEIGHT;

        set_pixel_f(canvas, projected_x[i], projected_y[i], 1.0f);
    }

    // Cube edges
    int edges[12][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };

    for (int i = 0; i < 12; i++) {
        int a = edges[i][0];
        int b = edges[i][1];
        draw_line_f(canvas, projected_x[a], projected_y[a], projected_x[b], projected_y[b], 1.0f);
    }

    // Write output to PGM file
    FILE* f = fopen("test_math_output.pgm", "w");
    fprintf(f, "P2\n%d %d\n255\n", WIDTH, HEIGHT);
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int val = (int)(fminf(canvas->pixels[y][x], 1.0f) * 255.0f);
            fprintf(f, "%d ", val);
        }
        fprintf(f, "\n");
    }
    fclose(f);

    destroy_canvas(canvas);
    return 0;
}
