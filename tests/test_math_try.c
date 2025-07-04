#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../include/canvas.h"
#include "../include/math3d.h"

// Save grayscale image (PGM format)
void canvas_write_pgm(canvas_t *canvas, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return;
    }

    fprintf(f, "P2\n%d %d\n255\n", canvas->width, canvas->height);

    for (int y = 0; y < canvas->height; y++) {
        for (int x = 0; x < canvas->width; x++) {
            int brightness = (int)(canvas->pixels[y][x] * 255.0f);
            if (brightness > 255) brightness = 255;
            fprintf(f, "%d ", brightness);
        }
        fprintf(f, "\n");
    }

    fclose(f);
}

// Apply 4x4 transform to vec3
vec3_t apply_transform(mat4_t m, vec3_t v) {
    vec3_t result;
    result.x = m.m[0]*v.x + m.m[4]*v.y + m.m[8]*v.z + m.m[12];
    result.y = m.m[1]*v.x + m.m[5]*v.y + m.m[9]*v.z + m.m[13];
    result.z = m.m[2]*v.x + m.m[6]*v.y + m.m[10]*v.z + m.m[14];
    return result;
}

int main() {
    canvas_t *canvas = create_canvas(400, 400);

    vec3_t cube[8] = {
        { -1, -1, -1 }, {  1, -1, -1 },
        {  1,  1, -1 }, { -1,  1, -1 },
        { -1, -1,  1 }, {  1, -1,  1 },
        {  1,  1,  1 }, { -1,  1,  1 }
    };

    // Reduce scale so the cube fits in the canvas
    float scale = 0.5f;

    mat4_t model = mat4_multiply(
        mat4_translate(0, 0, 0),
        mat4_multiply(
            mat4_rotate_xyz(0.4f, 0.8f, 0.2f),
            mat4_scale(scale, scale, scale)
        )
    );

    // Projected 2D points
    vec3_t transformed[8];
    float projected_x[8], projected_y[8];

    for (int i = 0; i < 8; i++) {
        vec3_t p = apply_transform(model, cube[i]);
        transformed[i] = p;

        // Orthographic projection to canvas space
        projected_x[i] = (p.x + 1.0f) * canvas->width / 2.0f;
        projected_y[i] = (1.0f - p.y) * canvas->height / 2.0f;

        set_pixel_f(canvas, projected_x[i], projected_y[i], 1.0f);
    }

    // Cube edges
    int edges[12][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},  // bottom
        {4, 5}, {5, 6}, {6, 7}, {7, 4},  // top
        {0, 4}, {1, 5}, {2, 6}, {3, 7}   // vertical
    };

    for (int i = 0; i < 12; i++) {
        int a = edges[i][0];
        int b = edges[i][1];
        draw_line_f(canvas, projected_x[a], projected_y[a], projected_x[b], projected_y[b], 1.0f);
    }

    canvas_write_pgm(canvas, "cube_canvas.pgm");
    destroy_canvas(canvas);

    printf("Saved cube_canvas.pgm using canvas system (scaled cube fits canvas!)\n");
    return 0;
}