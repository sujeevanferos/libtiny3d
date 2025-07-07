#include "../include/renderer.h" 
#include "../include/animation.h" 
#include <stdio.h>
#include <math.h>
#include <string.h> 

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Main demo: Shows two soccer balls, different sizes, self-rotating, 
// and moving in synced, looping circular paths using trigonometry.
int main() {
    int width = 900; 
    int height = 900;
    canvas_t* canvas = canvas_create(width, height);

    if (!canvas) {
        fprintf(stderr, "Failed to create canvas.\n");
        return 1;
    }

    model_t* soccer_ball_geom = generate_soccer_ball(); 
    if (!soccer_ball_geom) {
        fprintf(stderr, "Failed to generate soccer_ball geometry.\n");
        canvas_destroy(canvas);
        return 1;
    }

    light_t lights[1];
    lights[0].type = LIGHT_TYPE_DIRECTIONAL;
    lights[0].direction = vec3_create_cartesian(0.7f, 0.7f, -0.7f); 
    vec3_normalize(&lights[0].direction);
    int num_lights = 1;

    vec3_t eye = vec3_create_cartesian(0.0f, 0.0f, 7.5f); // Adjusted Z for new path sizes
    mat4_t view_matrix = mat4_translate(-eye.x, -eye.y, -eye.z);

    float aspect_ratio = (float)width / (float)height;
    float fov_y_rad = M_PI / 3.0f; 
    float near_plane = 0.1f;
    float far_plane = 100.0f;
    mat4_t projection_matrix = mat4_perspective(fov_y_rad, aspect_ratio, near_plane, far_plane);

    // --- Animation Setup ---
    int num_frames = 90; // 3 seconds at 30 FPS
    float total_animation_duration = 3.0f; 
    float time_step = total_animation_duration / (float)num_frames;
    float current_time = 0.0f;
    
    // --- Ball 1 Parameters ---
    mat4_t scale_matrix1 = mat4_scale(1.2f, 1.2f, 1.2f);
    int full_rotations1 = 2; // Ball 1 will make 2 full self-rotations over 3 seconds
    float self_rotation_speed1 = (full_rotations1 * 2.0f * M_PI) / total_animation_duration; 
    vec3_t self_rotation_axis1 = vec3_create_cartesian(0.1f, 1.0f, 0.05f);
    vec3_normalize(&self_rotation_axis1);
    float circular_path_radius1 = 2.0f; // Radius of its circular path
    float path_phase1 = 0.0f;           // Starting angle on its circular path

    // --- Ball 2 Parameters ---
    mat4_t scale_matrix2 = mat4_scale(0.9f, 0.9f, 0.9f); 
    int full_rotations2 = 1; // Ball 2 will make 1 full self-rotation over 3 seconds
    float self_rotation_speed2 = (full_rotations2 * 2.0f * M_PI) / total_animation_duration; 
    vec3_t self_rotation_axis2 = vec3_create_cartesian(1.0f, 0.5f, -0.1f);
    vec3_normalize(&self_rotation_axis2);
    float circular_path_radius2 = 1.2f; // Smaller circular path
    float path_phase2 = M_PI;           // Starts on the opposite side of its path's origin compared to ball1 if radii were same

    float viewport_radius = fminf(width, height) / 2.0f * 0.98f; 
    float line_thickness = 1.0f; 

    printf("Starting animation: %d frames (TWO soccer balls, trigonometric circular paths, self-rotating)...\n", num_frames);

    for (int frame = 0; frame < num_frames; ++frame) {
        canvas_clear(canvas, 0.02f); 
        canvas_set_circular_viewport(canvas, viewport_radius);
        
        // --- BALL 1 ---
        // Self-rotation
        float current_self_rot1 = fmodf(current_time * self_rotation_speed1, 2.0f * M_PI);
        quat_t rot_q1 = quat_from_axis_angle(self_rotation_axis1, current_self_rot1);
        mat4_t rot_m1 = quat_to_mat4(rot_q1);
        mat4_t base_model1 = mat4_multiply(&rot_m1, &scale_matrix1);
        // Circular Path Translation
        float angle_on_circle1 = fmodf(path_phase1 + (current_time / total_animation_duration) * 2.0f * M_PI, 2.0f * M_PI);
        float path_x1 = circular_path_radius1 * cosf(angle_on_circle1);
        float path_z1 = circular_path_radius1 * sinf(angle_on_circle1); // Path in XZ plane
        mat4_t path_translate_m1 = mat4_translate(path_x1, 0.0f, path_z1);
        mat4_t model_matrix1 = mat4_multiply(&path_translate_m1, &base_model1);
        render_wireframe(canvas, soccer_ball_geom, &model_matrix1, &view_matrix, &projection_matrix, lights, num_lights, viewport_radius, line_thickness);

        // --- BALL 2 ---
        // Self-rotation
        float current_self_rot2 = fmodf(current_time * self_rotation_speed2, 2.0f * M_PI);
        quat_t rot_q2 = quat_from_axis_angle(self_rotation_axis2, current_self_rot2);
        mat4_t rot_m2 = quat_to_mat4(rot_q2);
        mat4_t base_model2 = mat4_multiply(&rot_m2, &scale_matrix2);
        // Circular Path Translation
        float angle_on_circle2 = fmodf(path_phase2 + (current_time / total_animation_duration) * 2.0f * M_PI, 2.0f * M_PI);
        // To make paths distinct, Ball 2 could orbit in XY plane or a different XZ radius/center
        // For now, also XZ plane, different radius, different phase.
        float path_x2 = circular_path_radius2 * cosf(angle_on_circle2);
        float path_z2 = circular_path_radius2 * sinf(angle_on_circle2); 
        mat4_t path_translate_m2 = mat4_translate(path_x2, 0.0f, path_z2); 
        mat4_t model_matrix2 = mat4_multiply(&path_translate_m2, &base_model2);
        render_wireframe(canvas, soccer_ball_geom, &model_matrix2, &view_matrix, &projection_matrix, lights, num_lights, viewport_radius, line_thickness);
        
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

    model_destroy(soccer_ball_geom);
    canvas_destroy(canvas);
    return 0;
}

