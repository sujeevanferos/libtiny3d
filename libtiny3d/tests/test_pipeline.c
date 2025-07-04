#include "renderer.h" // Includes all necessary headers like math3d.h, canvas.h
#include <stdio.h>
#include <math.h> // For M_PI if needed

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

// Helper to print projected_vertex_t for debugging/verification
void print_projected_vertex(const char* name, const projected_vertex_t* pv) {
    printf("%s: Screen(x=%.2f, y=%.2f, z_cam=%.2f), Clipped=%d\n",
           name, pv->position_screen.x, pv->position_screen.y, pv->position_screen.z, pv->is_clipped);
}

int main() {
    printf("--- Rendering Pipeline Test ---\n");

    // 1. Define a simple model (e.g., a single triangle or a quad)
    // For this test, let's just project a few known vertices without a full model_t.
    vec3_t tri_verts[3];
    tri_verts[0] = vec3_create_cartesian(0.0f, 0.5f, 0.0f);  // Top
    tri_verts[1] = vec3_create_cartesian(-0.5f, -0.5f, 0.0f); // Bottom-left
    tri_verts[2] = vec3_create_cartesian(0.5f, -0.5f, 0.0f);  // Bottom-right

    // 2. Setup matrices
    mat4_t model_matrix = mat4_identity(); // Simple identity model matrix (model at origin)
    // mat4_t model_matrix = mat4_translate(0.0f, 0.0f, -2.0f); // Move triangle back slightly

    vec3_t eye = vec3_create_cartesian(0.0f, 0.0f, 3.0f); // Camera at (0,0,3) looking towards -Z
    mat4_t view_matrix = mat4_translate(-eye.x, -eye.y, -eye.z);

    int screen_width = 200;
    int screen_height = 150;
    float aspect_ratio = (float)screen_width / (float)screen_height;
    float fov_y_rad = M_PI / 2.0f; // 90 degrees FOV for dramatic perspective
    float near_plane = 0.1f;
    float far_plane = 10.0f;
    mat4_t projection_matrix = mat4_perspective(fov_y_rad, aspect_ratio, near_plane, far_plane);

    printf("\nTest Case 1: Projecting a triangle\n");
    printf("Screen Dimensions: %dx%d\n", screen_width, screen_height);
    printf("Model Matrix: Identity\n");
    // print_mat4("View Matrix", &view_matrix);
    // print_mat4("Projection Matrix", &projection_matrix);


    projected_vertex_t pv[3];
    for (int i = 0; i < 3; ++i) {
        char v_name[10];
        sprintf(v_name, "Vert %d", i);
        pv[i] = project_vertex(tri_verts[i], &model_matrix, &view_matrix, &projection_matrix, screen_width, screen_height);
        print_projected_vertex(v_name, &pv[i]);
    }

    // Expected values (approximate, would need careful manual calculation or a reference implementation run)
    // For Vert 0 (0, 0.5, 0) -> World (0, 0.5, 0) -> Cam (0, 0.5, -3)
    // NDC_y for 0.5 at Z=-3, with 90deg FOV (tan(45)=1): top of frustum at Z=-3 is y=3. So 0.5/3 = 0.166 in NDC.
    // Screen Y for NDC 0.166: (1 - 0.166) * 0.5 * 150 = 0.834 * 75 = 62.55
    // Screen X for NDC 0: (0+1)*0.5*200 = 100
    // So Vert 0 should be near (100, 62.55)

    // Vert 1 (-0.5, -0.5, 0) -> World (-0.5, -0.5, 0) -> Cam (-0.5, -0.5, -3)
    // NDC_x for -0.5 at Z=-3, aspect=200/150=1.333. Right of frustum at Z=-3 is x=3*aspect_ratio_of_fov_x_not_screen.
    // Perspective projection: x_ndc = (x_cam / -z_cam) * (1 / tan(fovX/2))
    // x_clip = P[0][0]*x_c + P[0][2]*z_c = (1/tan(fovX/2))*x_c + ...
    // P = mat4_perspective(fov_y_rad (PI/2 -> tan(PI/4)=1), aspect (4/3), n, f);
    // P[0][0] = 1 / (aspect * tan(fov_y/2)) = 1 / ( (4/3) * 1 ) = 3/4 = 0.75
    // P[1][1] = 1 / tan(fov_y/2) = 1 / 1 = 1.0
    // x_clip = 0.75 * x_c + ... (no, P[0] is x_scale, P[2] is x_offset from center)
    // x_h = P[0]*x_c + P[8]*z_c (if P[0] is m00, P[8] is m20 for col-major)
    // Corrected perspective matrix elements from mat4_perspective:
    // top = n * tan(fovy/2) = 0.1 * 1 = 0.1
    // right = top * aspect = 0.1 * 4/3 = 0.1333
    // P.m[0] = (2n)/(r-l) = n/r = 0.1 / 0.1333 = 0.75
    // P.m[5] = (2n)/(t-b) = n/t = 0.1 / 0.1 = 1.0
    //
    // V0 (0, 0.5, 0) -> Cam (0, 0.5, -3)
    // Clip: x_h=0, y_h=0.5, z_h=..., w_h=3
    // NDC: x_n=0, y_n=0.5/3=0.166
    // Screen: sx=(0+1)*0.5*200=100, sy=(1-0.166)*0.5*150 = 62.5
    //
    // V1 (-0.5, -0.5, 0) -> Cam (-0.5, -0.5, -3)
    // Clip: x_h = 0.75*(-0.5) = -0.375. y_h = 1.0*(-0.5) = -0.5. w_h=3
    // NDC: x_n = -0.375/3 = -0.125. y_n = -0.5/3 = -0.166
    // Screen: sx=(-0.125+1)*0.5*200 = 87.5. sy=(1-(-0.166))*0.5*150 = 1.166 * 75 = 87.5

    // Simple test on clipping a pixel (if we had a canvas)
    // canvas_t* test_canvas = canvas_create(screen_width, screen_height);
    // if (test_canvas) {
    //     canvas_set_circular_viewport(test_canvas, screen_height / 3.0f); // radius = 50
    //     // Assuming _canvas_is_pixel_in_circular_viewport is made non-static for testing or use public API
    //     // int R = screen_height / 3;
    //     // int cx = screen_width / 2;
    //     // int cy = screen_height / 2;
    //     // printf("Pixel (cx, cy) in viewport: %d (expected 1)\n", _canvas_is_pixel_in_circular_viewport(test_canvas, cx, cy));
    //     // printf("Pixel (cx+R+1, cy) in viewport: %d (expected 0)\n", _canvas_is_pixel_in_circular_viewport(test_canvas, cx+R+1, cy));
    //     canvas_destroy(test_canvas);
    // }


    printf("\nPipeline test finished. Manual verification of coordinates needed.\n");
    return 0;
}
