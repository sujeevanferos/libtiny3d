// Rotating soccer ball clipped to a circle
#include "../include/renderer.h"
#include "../include/animation.h" // For bezier_cubic
#include <stdio.h>
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Main demo: Show a single soccer ball rotating on its axis AND moving in a circular path.
int main() {
    int width = 1024; // Slightly wider canvas
    int height = 1024;
    canvas_t* canvas = canvas_create(width, height);

    if (!canvas) {
        fprintf(stderr, "Failed to create canvas.\n");
        return 1;
    }

    // --- Model ---
    model_t* soccer_ball = generate_soccer_ball();
    if (!soccer_ball) {
        fprintf(stderr, "Failed to generate soccer_ball model.\n");
        canvas_destroy(canvas);
        return 1;
    }

    // --- Lighting ---
    light_t lights[1];
    lights[0].type = LIGHT_TYPE_DIRECTIONAL;
    // Light direction primarily from top-left-front
    lights[0].direction = vec3_create_cartesian(0.5f, 0.7f, -1.0f);
    vec3_normalize(&lights[0].direction);
    int num_lights = 1;

    // --- View and Projection Matrices ---
    // Camera looking directly at the origin to center the XZ plane animation.
    vec3_t eye = vec3_create_cartesian(0.0f, 0.0f, 7.0f); // Camera Y at 0, looking straight.
    mat4_t view_matrix = mat4_translate(-eye.x, -eye.y, -eye.z);
    // Removed x-axis rotation for a direct view of the origin in the screen center.
    // mat4_t view_rot_x = mat4_rotate_x(-M_PI / 12.0f);
    // view_matrix = mat4_multiply(&view_rot_x, &view_matrix);


    float aspect_ratio = (float)width / (float)height;
    float fov_y_rad = M_PI / 3.0f; // ~60 degrees FOV
    float near_plane = 0.1f;
    float far_plane = 100.0f;
    mat4_t projection_matrix = mat4_perspective(fov_y_rad, aspect_ratio, near_plane, far_plane);

    // --- Animation Setup ---
    int num_frames = 120; // e.g., 4 seconds at 30 FPS
    float total_animation_duration = 4.0f; // Duration for one full loop of everything
    float time_step = total_animation_duration / (float)num_frames;

    float current_time = 0.0f;

    // Self-Rotation parameters
    float self_rotation_speed = M_PI; // Radians per second (e.g., 180 deg/sec)
    vec3_t self_rotation_axis = vec3_create_cartesian(0.1f, 1.0f, 0.05f); // Tilted axis
    vec3_normalize(&self_rotation_axis);

    // Circular Path Translation parameters
    float circular_path_radius = 1.8f;
    float circular_path_period = total_animation_duration; // Path completes one loop in total_animation_duration

    // Static scale for the soccer ball
    mat4_t scale_matrix = mat4_scale(1.7f, 1.7f, 1.7f); // Adjusted scale

    float viewport_radius = fminf(width, height) / 2.0f * 0.98f;
    float line_thickness = 1.0f;

    printf("Starting animation: %d frames (soccer ball rotating and moving in a circle)...\n", num_frames);

    for (int frame = 0; frame < num_frames; ++frame) {
        canvas_clear(canvas, 0.02f); // Very dark background
        canvas_set_circular_viewport(canvas, viewport_radius);

        // 1. Calculate self-rotation
        float current_self_rotation_angle = fmodf(current_time * self_rotation_speed, 2.0f * M_PI);
        quat_t rotation_q = quat_from_axis_angle(self_rotation_axis, current_self_rotation_angle);
        mat4_t rotation_m = quat_to_mat4(rotation_q);

        // Base model matrix (scale then self-rotate)
        mat4_t base_model_matrix = mat4_multiply(&rotation_m, &scale_matrix);

        // 2. Calculate circular path translation
        float angle_on_circle = fmodf((current_time / circular_path_period) * 2.0f * M_PI, 2.0f * M_PI);
        float path_x = circular_path_radius * cosf(angle_on_circle);
        float path_z = circular_path_radius * sinf(angle_on_circle); // Moving in XZ plane
        // float path_y = 0.0f; // Or could add a vertical component, e.g. path_y = 0.5f * sinf(angle_on_circle * 2.0f); for a bob
        mat4_t path_translate_m = mat4_translate(path_x, 0.0f, path_z);

        // Final model matrix: Apply path translation to the (rotated and scaled) base model
        mat4_t model_matrix = mat4_multiply(&path_translate_m, &base_model_matrix);

        // Render
        render_wireframe(canvas, soccer_ball, &model_matrix, &view_matrix, &projection_matrix, lights, num_lights, viewport_radius, line_thickness);

        char frame_filename[100];
        sprintf(frame_filename, "build/frame_%04d.pgm", frame);
        if (canvas_save_to_pgm(canvas, frame_filename) != 0) {
            fprintf(stderr, "Failed to save frame %s\n", frame_filename);
        }

        if (frame % (num_frames/10) == 0 || frame == num_frames -1) {
             printf("Rendered frame %d / %d to %s\n", frame + 1, num_frames, frame_filename);
        }
        current_time += time_step;
    }

    printf("Animation rendering finished. Output frames are in 'build/' directory.\n");

    model_destroy(soccer_ball);
    canvas_destroy(canvas);
    return 0;
}
