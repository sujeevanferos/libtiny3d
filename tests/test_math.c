#include "math3d.h"
#include <stdio.h>
#include <math.h> // For M_PI if available, and other math functions
#include "canvas.h" // For PGM output
// Note: stdio.h and math.h were already included above, removing redundant includes.

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

// Helper to print a vec3_t
void print_vec3(const char* name, const vec3_t* v) {
    printf("%s (Cartesian): (%.2f, %.2f, %.2f)\n", name, v->x, v->y, v->z);
    // printf("%s (Spherical): (r=%.2f, th=%.2f, ph=%.2f)\n", name, v->r, v->theta * 180/M_PI, v->phi * 180/M_PI);
}

// Helper to print a mat4_t (column-major)
void print_mat4(const char* name, const mat4_t* m) {
    printf("%s:\n", name);
    for (int r = 0; r < 4; ++r) {
        printf("  [ %.2f %.2f %.2f %.2f ]\n", m->m[0*4+r], m->m[1*4+r], m->m[2*4+r], m->m[3*4+r]);
    }
}

int main() {
    printf("--- 3D Math Test ---\n");

    // 1. Test vec3_t creation and synchronization
    printf("\n--- Vector Tests ---\n");
    vec3_t v1 = vec3_create_cartesian(1.0f, 1.0f, 1.0f);
    print_vec3("v1 (Cartesian set)", &v1);
    printf("v1 Spherical: r=%.2f, theta=%.2f rad, phi=%.2f rad\n", v1.r, v1.theta, v1.phi);

    vec3_t v2 = vec3_from_spherical(2.0f, M_PI/4.0f, M_PI/2.0f); // r=2, theta=45deg, phi=90deg
    print_vec3("v2 (Spherical set)", &v2);
     printf("v2 Cartesian: x=%.2f, y=%.2f, z=%.2f\n", v2.x, v2.y, v2.z);

    vec3_set_cartesian(&v2, 1.f,0.f,0.f);
    print_vec3("v2 (Cartesian updated)", &v2);
    printf("v2 Spherical: r=%.2f, theta=%.2f rad, phi=%.2f rad\n", v2.r, v2.theta, v2.phi);


    // Test normalization
    vec3_t v_norm = vec3_create_cartesian(3.0f, 4.0f, 0.0f);
    print_vec3("v_norm (original)", &v_norm);
    vec3_normalize(&v_norm);
    print_vec3("v_norm (normalized)", &v_norm);

    vec3_t v_norm_fast = vec3_create_cartesian(1.0f, 2.0f, 3.0f);
    print_vec3("v_norm_fast (original)", &v_norm_fast);
    vec3_normalize_fast(&v_norm_fast);
    print_vec3("v_norm_fast (normalized fast)", &v_norm_fast);

    // Test SLERP
    vec3_t slerp_a = vec3_create_cartesian(1,0,0); // Normalized by creation if r=1
    vec3_normalize(&slerp_a); // ensure
    vec3_t slerp_b = vec3_create_cartesian(0,1,0);
    vec3_normalize(&slerp_b); // ensure
    print_vec3("slerp_a", &slerp_a);
    print_vec3("slerp_b", &slerp_b);
    vec3_t slerp_res = vec3_slerp(&slerp_a, &slerp_b, 0.5f);
    print_vec3("slerp_res (t=0.5)", &slerp_res); // Should be (0.707, 0.707, 0)


    // 2. Define a simple cube (vertices)
    // Centered at origin, side length 2 (from -1 to +1)
    vec3_t cube_vertices[8] = {
        vec3_create_cartesian(-1.0f, -1.0f, -1.0f), // 0: Bottom-left-back
        vec3_create_cartesian( 1.0f, -1.0f, -1.0f), // 1: Bottom-right-back
        vec3_create_cartesian( 1.0f,  1.0f, -1.0f), // 2: Top-right-back
        vec3_create_cartesian(-1.0f,  1.0f, -1.0f), // 3: Top-left-back
        vec3_create_cartesian(-1.0f, -1.0f,  1.0f), // 4: Bottom-left-front
        vec3_create_cartesian( 1.0f, -1.0f,  1.0f), // 5: Bottom-right-front
        vec3_create_cartesian( 1.0f,  1.0f,  1.0f), // 6: Top-right-front
        vec3_create_cartesian(-1.0f,  1.0f,  1.0f)  // 7: Top-left-front
    };
    vec3_t transformed_vertices[8];

    printf("\n--- Matrix & Transformation Tests ---\n");
    // 3. Create transformation matrices
    mat4_t scale_matrix = mat4_scale(0.5f, 0.5f, 0.5f); // Scale by 0.5
    // print_mat4("Scale Matrix", &scale_matrix); // Reduced verbosity for this version

    mat4_t rotation_matrix = mat4_rotate_xyz(M_PI / 4.0f, M_PI / 4.0f, 0.0f); // Rotate 45 deg around X, then 45 deg around Y
    // print_mat4("Rotation Matrix", &rotation_matrix);

    mat4_t translation_matrix = mat4_translate(0.0f, 0.0f, -5.0f); // Move 5 units away from camera (into -Z)
    // print_mat4("Translation Matrix", &translation_matrix);

    mat4_t model_matrix_rs = mat4_multiply(&rotation_matrix, &scale_matrix);
    mat4_t model_matrix = mat4_multiply(&translation_matrix, &model_matrix_rs);
    // print_mat4("Model Matrix (T*R*S)", &model_matrix);

    mat4_t projection_matrix = mat4_perspective(M_PI / 2.0f, 1.0f, 1.0f, 100.0f); // 90deg FOV, 1:1 aspect, near=1, far=100
    // print_mat4("Projection Matrix", &projection_matrix);
    
    mat4_t mvp_matrix = mat4_multiply(&projection_matrix, &model_matrix);
    // print_mat4("MVP Matrix (Projection * Model)", &mvp_matrix);

    // printf("\n--- Transformed Cube Vertices (Clip Space before perspective divide) ---\n"); // Reduced verbosity
    for (int i = 0; i < 8; ++i) {
        transformed_vertices[i] = mat4_transform_point(&mvp_matrix, &cube_vertices[i]);
        // char v_name[10];
        // sprintf(v_name, "V%d_clip", i);
        // print_vec3(v_name, &transformed_vertices[i]); // Reduced verbosity
    }
    
    printf("\n--- Quaternion Tests ---\n");
    vec3_t axis = vec3_create_cartesian(0,1,0); 
    vec3_normalize(&axis);
    quat_t q_rot = quat_from_axis_angle(axis, M_PI/2.0f); 
    printf("Quaternion for 90deg rot around Y: (%.2f, %.2f, %.2f, %.2f)\n", q_rot.x, q_rot.y, q_rot.z, q_rot.w);
    
    mat4_t rot_from_quat = quat_to_mat4(q_rot);
    print_mat4("Rotation Matrix from Quaternion", &rot_from_quat); 
    
    quat_t q_ident = {0,0,0,1}; 
    quat_t q_slerp_res = quat_slerp(q_ident, q_rot, 0.5f); 
    printf("SLERP(ident, q_rot, 0.5): (%.2f, %.2f, %.2f, %.2f)\n", q_slerp_res.x, q_slerp_res.y, q_slerp_res.z, q_slerp_res.w);
    mat4_t rot_from_slerp_quat = quat_to_mat4(q_slerp_res);
    print_mat4("Matrix from SLERP'd Quaternion (45 deg rot around Y)", &rot_from_slerp_quat);

    printf("\n--- Visualizing Transformed Cube ---");
    int canvas_width = 300;
    int canvas_height = 200;
    canvas_t* viz_canvas = canvas_create(canvas_width, canvas_height);
    if (!viz_canvas) {
        fprintf(stderr, "Failed to create canvas for cube visualization.\n");
    } else {
        canvas_clear(viz_canvas, 0.1f); 

        int cube_edges[12][2] = {
            {0,1}, {1,2}, {2,3}, {3,0}, 
            {4,5}, {5,6}, {6,7}, {7,4}, 
            {0,4}, {1,5}, {2,6}, {3,7}  
        };

        vec3_t screen_coords[8];
        for(int i=0; i<8; ++i) {
            screen_coords[i].x = (transformed_vertices[i].x + 1.0f) * 0.5f * canvas_width;
            screen_coords[i].y = (1.0f - transformed_vertices[i].y) * 0.5f * canvas_height;
        }

        float line_thickness = 1.5f;
        float line_intensity = 1.0f;

        for (int i = 0; i < 12; ++i) {
            vec3_t v0_screen = screen_coords[cube_edges[i][0]];
            vec3_t v1_screen = screen_coords[cube_edges[i][1]];
            draw_line_f(viz_canvas, v0_screen.x, v0_screen.y, v1_screen.x, v1_screen.y, line_thickness, line_intensity);
        }
        
        const char* pgm_filename = "build/task2_math_cube_output.pgm";
        if (canvas_save_to_pgm(viz_canvas, pgm_filename) == 0) {
            printf("\nTransformed cube visualization saved to %s\n", pgm_filename);
        } else {
            fprintf(stderr, "\nFailed to save cube visualization PGM.\n");
        }
        canvas_destroy(viz_canvas);
    }

    printf("\nMath test finished.\n");
    return 0;
}

