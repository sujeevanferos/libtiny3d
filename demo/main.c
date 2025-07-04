#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h> // for usleep

#include "../include/canvas.h"
#include "../include/math3d.h"
#include "../include/renderer.h"
#include "../include/soccer_loader.h"
#include "../include/lighting.h"
#include "../include/animation.h"

#define WIDTH 512
#define HEIGHT 512
#define FRAMES 60

int main() {
    mesh_t ball = load_obj("soccer_ball.obj");

    // Light direction (towards the camera)
    vec3_t light_dir = vec3_from_cartesian(0.0f, 0.0f, -1.0f);

    // Bezier control points for animation
    vec3_t p0 = vec3_from_cartesian(0.0f, 0.0f, -3.5f);
    vec3_t p1 = vec3_from_cartesian(1.0f, 1.0f, -2.0f);
    vec3_t p2 = vec3_from_cartesian(-1.0f, -1.0f, -2.0f);
    vec3_t p3 = vec3_from_cartesian(0.0f, 0.0f, -3.5f);

    for (int frame = 0; frame < FRAMES; frame++) {
        char filename[64];
        sprintf(filename, "frame_%03d.pgm", frame);

        canvas_t* canvas = create_canvas(WIDTH, HEIGHT);

        float angle = frame * 2 * M_PI / FRAMES;
        float t = (float)frame / (FRAMES - 1);

        mat4_t model = mat4_mul(
            mat4_rotate_xyz(angle, angle * 0.5f, 0.0f),
            mat4_scale(1.2f, 1.2f, 1.2f)
        );

        vec3_t cam_pos = bezier(p0, p1, p2, p3, t);
        mat4_t view = mat4_translate(cam_pos.x, cam_pos.y, cam_pos.z);
        mat4_t proj = mat4_frustum_asymmetric(-1, 1, -1, 1, 1.0f, 10.0f);
        mat4_t mvp = mat4_mul(proj, mat4_mul(view, model));

        for (int i = 0; i < ball.edge_count; i++) {
            vec3_t v0 = mat4_mul_vec3(mvp, ball.vertices[ball.edges[i][0]]);
            vec3_t v1 = mat4_mul_vec3(mvp, ball.vertices[ball.edges[i][1]]);

            float sx0 = (v0.x + 1.0f) * 0.5f * WIDTH;
            float sy0 = (1.0f - v0.y) * 0.5f * HEIGHT;
            float sx1 = (v1.x + 1.0f) * 0.5f * WIDTH;
            float sy1 = (1.0f - v1.y) * 0.5f * HEIGHT;

            if (clip_to_circular_viewport(canvas, sx0, sy0) &&
                clip_to_circular_viewport(canvas, sx1, sy1)) {

                vec3_t edge_dir = vec3_from_cartesian(
                    ball.vertices[ball.edges[i][1]].x - ball.vertices[ball.edges[i][0]].x,
                    ball.vertices[ball.edges[i][1]].y - ball.vertices[ball.edges[i][0]].y,
                    ball.vertices[ball.edges[i][1]].z - ball.vertices[ball.edges[i][0]].z
                );

                float intensity = compute_lambert_intensity(edge_dir, light_dir);
                draw_line_f(canvas, sx0, sy0, sx1, sy1, intensity);
            }
        }

        // Write to .pgm
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
        destroy_canvas(canvas);
    }

    free(ball.vertices);
    free(ball.edges);
    return 0;
}