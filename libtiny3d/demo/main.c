#include "renderer.h" // Includes canvas.h, math3d.h, lighting.h
#include "animation.h"// For bezier_cubic and animation structures
#include <stdio.h>
#include <math.h>
#include <string.h> // For sprintf

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Main demo for Task 4: Multiple lit wireframe shapes moving smoothly in sync.
int main() {
    int width = 500;
    int height = 500;
    canvas_t* canvas = canvas_create(width, height);

    if (!canvas) {
        fprintf(stderr, "Failed to create canvas.\n");
        return 1;
    }

    // --- Models ---
    model_t* model1 = generate_soccer_ball(); // Currently a cube
    if (!model1) {
        fprintf(stderr, "Failed to generate model1.\n");
        canvas_destroy(canvas);
        return 1;
    }
    // Create a second model, maybe a scaled version or different orientation
    model_t* model2 = generate_soccer_ball(); // Also a cube
     if (!model2) {
        fprintf(stderr, "Failed to generate model2.\n");
        model_destroy(model1);
        canvas_destroy(canvas);
        return 1;
    }


    // --- Lighting ---
    light_t lights[1];
    lights[0].type = LIGHT_TYPE_DIRECTIONAL;
    lights[0].direction = vec3_create_cartesian(0.8f, 0.8f, -1.0f);
    vec3_normalize(&lights[0].direction);
    int num_lights = 1;

    // --- View and Projection Matrices (static for this demo) ---
    vec3_t eye = vec3_create_cartesian(0.0f, 2.0f, 8.0f); // Camera further away and slightly elevated
    mat4_t view_matrix = mat4_translate(-eye.x, -eye.y, -eye.z); // Simple view
    // To make it look at origin: mat4_look_at would be better.
    // For now, a slight rotation to point camera downwards if it's elevated
    mat4_t view_rot = mat4_rotate_x(-M_PI/10.0f); // Rotate camera view down
    view_matrix = mat4_multiply(&view_rot, &view_matrix);


    float aspect_ratio = (float)width / (float)height;
    float fov_y_rad = M_PI / 3.0f;
    float near_plane = 0.1f;
    float far_plane = 100.0f;
    mat4_t projection_matrix = mat4_perspective(fov_y_rad, aspect_ratio, near_plane, far_plane);

    // --- Animation Setup ---
    // Path 1 (e.g., circular in XZ plane)
    bezier_animation_path_t path1;
    path1.control_points[0] = vec3_create_cartesian(-2.0f, 0.0f, 0.0f); // P0
    path1.control_points[1] = vec3_create_cartesian(-2.0f, 0.0f, 2.0f); // P1 (handle for P0)
    path1.control_points[2] = vec3_create_cartesian( 2.0f, 0.0f, 2.0f); // P2 (handle for P3)
    path1.control_points[3] = vec3_create_cartesian( 2.0f, 0.0f, 0.0f); // P3
    // To make it loop somewhat smoothly, P3 could go back to P0, and handles adjusted.
    // For a simple back and forth:
    // P0_return = P3, P1_return = P2, P2_return = P1, P3_return = P0
    path1.duration = 5.0f; // 5 seconds for one segment

    // Path 2 (e.g., vertical oscillation or different planar path)
    bezier_animation_path_t path2;
    path2.control_points[0] = vec3_create_cartesian(1.5f, -1.0f, -1.0f);
    path2.control_points[1] = vec3_create_cartesian(1.5f,  1.0f, -1.0f);
    path2.control_points[2] = vec3_create_cartesian(1.5f,  1.0f, -1.0f); // Can make P1=P2 for linear segment if desired
    path2.control_points[3] = vec3_create_cartesian(1.5f, -1.0f, -1.0f); // Simple up and down
    path2.duration = 2.5f; // 2.5 seconds for one segment


    // --- Animation Loop ---
    int num_frames = 10; // Reduced for testing (e.g., 5 seconds at 30 FPS -> 150 frames)
    float total_duration_for_loop_sync = 5.0f; // LCM of path durations for full sync, or just run for N frames
    float time_step = total_duration_for_loop_sync / (float)num_frames; // Adjust if num_frames changes for desired speed
    // If num_frames is small, time_step will be large, animation will be fast.
    // Let's set time_step based on a desired FPS, e.g., 30 FPS.
    // float target_fps = 30.0f;
    // time_step = 1.0f / target_fps;
    // num_frames = (int)(total_duration_for_loop_sync * target_fps);
    // For quick test:
    time_step = 0.1f; // Render 10 frames, 1 second of animation if duration is 1s.
                      // With total_duration_for_loop_sync = 5.0, this is 10 frames over 1s of its cycle.
                      // Let's make num_frames drive the total time simulated for this test run.

    float current_time = 0.0f;

    float viewport_radius = fminf(width, height) / 2.0f * 0.90f;
    float line_thickness = 1.5f;

    // Base rotations for the objects (static)
    quat_t base_rot_q1 = quat_from_axis_angle(vec3_create_cartesian(0,1,0), M_PI/4.0f);
    mat4_t base_rot_m1 = quat_to_mat4(base_rot_q1);

    quat_t base_rot_q2 = quat_from_axis_angle(vec3_create_cartesian(1,0,0), M_PI/3.0f);
    mat4_t base_rot_m2 = quat_to_mat4(base_rot_q2);
    mat4_t scale_m2 = mat4_scale(0.7f,0.7f,0.7f); // Make model2 smaller
    base_rot_m2 = mat4_multiply(&base_rot_m2, &scale_m2);


    printf("Starting animation rendering: %d frames...\n", num_frames);

    for (int frame = 0; frame < num_frames; ++frame) {
        canvas_clear(canvas, 0.05f); // Clear canvas (dark background)
        canvas_set_circular_viewport(canvas, viewport_radius);

        // --- Object 1 Animation ---
        float t1_param = fmodf(current_time, path1.duration) / path1.duration;
        // For ping-pong loop on a single path segment:
        if ((int)(current_time / path1.duration) % 2 != 0) { // If on an "odd" segment number, reverse t
            t1_param = 1.0f - t1_param;
        }
        vec3_t pos1 = bezier_cubic(path1.control_points[0], path1.control_points[1], path1.control_points[2], path1.control_points[3], t1_param);
        mat4_t trans_m1 = mat4_translate(pos1.x, pos1.y, pos1.z);
        mat4_t model_matrix1 = mat4_multiply(&trans_m1, &base_rot_m1); // Apply translation to base rotated model

        render_wireframe(canvas, model1, &model_matrix1, &view_matrix, &projection_matrix, lights, num_lights, viewport_radius, line_thickness);

        // --- Object 2 Animation ---
        float t2_param = fmodf(current_time, path2.duration) / path2.duration;
        if ((int)(current_time / path2.duration) % 2 != 0) {
            t2_param = 1.0f - t2_param;
        }
        vec3_t pos2 = bezier_cubic(path2.control_points[0], path2.control_points[1], path2.control_points[2], path2.control_points[3], t2_param);
        mat4_t trans_m2 = mat4_translate(pos2.x, pos2.y, pos2.z);
        mat4_t model_matrix2 = mat4_multiply(&trans_m2, &base_rot_m2);

        render_wireframe(canvas, model2, &model_matrix2, &view_matrix, &projection_matrix, lights, num_lights, viewport_radius, line_thickness);

        // Save frame
        char frame_filename[100];
        sprintf(frame_filename, "build/frame_%04d.pgm", frame);
        if (canvas_save_to_pgm(canvas, frame_filename) != 0) {
            fprintf(stderr, "Failed to save frame %s\n", frame_filename);
            // Decide if to break or continue
        }

        if (frame % (num_frames/10) == 0 || frame == num_frames -1) {
             printf("Rendered frame %d / %d to %s\n", frame + 1, num_frames, frame_filename);
        }

        current_time += time_step;
    }

    printf("Animation rendering finished. Output frames are in 'build/' directory.\n");

    model_destroy(model1);
    model_destroy(model2);
    canvas_destroy(canvas);
    return 0;
}
