#include "canvas.h"   // For canvas operations
#include "math3d.h"   // For M_PI and math functions like cosf, sinf (though standard math.h is also fine)
#include <stdio.h>    // For printf, fprintf
#include <math.h>     // For M_PI (ensure it's available), cosf, sinf, fminf

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

int main() {
    printf("--- Task 1 Clock Face Demo Test ---\n");

    int width = 400;
    int height = 300;
    canvas_t* canvas = canvas_create(width, height);

    if (!canvas) {
        fprintf(stderr, "Failed to create canvas for Task 1 demo.\n");
        return 1;
    }

    // Clear canvas to black
    canvas_clear(canvas, 0.0f);

    float center_x = width / 2.0f;
    float center_y = height / 2.0f;
    float line_length = fminf(width, height) / 2.5f;
    float line_thickness = 2.0f; // Standard thickness
    float default_intensity = 1.0f; // Full intensity for lines

    printf("Canvas created: %dx%d for clock face demo.\n", width, height);
    printf("Drawing clock face lines from center (%.1f, %.1f) with length %.1f, thickness %.1f\n",
           center_x, center_y, line_length, line_thickness);

    int num_lines = 360 / 15; // 24 lines for 15-degree steps
    for (int i = 0; i < num_lines; ++i) {
        float angle_deg = i * 15.0f;
        float angle_rad = angle_deg * (M_PI / 180.0f);

        float x1 = center_x;
        float y1 = center_y;
        float x2 = center_x + line_length * cosf(angle_rad);
        float y2 = center_y + line_length * sinf(angle_rad);

        // printf("Drawing line %d (%.0f deg): from (%.1f, %.1f) to (%.1f, %.1f)\n", i, angle_deg, x1, y1, x2, y2);
        draw_line_f(canvas, x1, y1, x2, y2, line_thickness, default_intensity);
    }

    // Example of a thicker line (e.g., for 12 o'clock)
    // float twelve_oclock_angle_rad = 90.0f * (M_PI / 180.0f); // 0 degrees is to the right, 90 is up.
    // draw_line_f(canvas, center_x, center_y,
    //             center_x + line_length * cosf(twelve_oclock_angle_rad),
    //             center_y + line_length * sinf(twelve_oclock_angle_rad),
    //             line_thickness * 2.0f, // Make it thicker
    //             default_intensity);


    const char* output_filename = "build/task1_clock_output.pgm";
    if (canvas_save_to_pgm(canvas, output_filename) == 0) {
        printf("Task 1 Clock Face Demo saved to %s\n", output_filename);
    } else {
        fprintf(stderr, "Failed to save Task 1 Clock Face Demo to %s\n", output_filename);
    }

    canvas_destroy(canvas);
    printf("Task 1 Clock Face Demo Test finished.\n");
    return 0;
}
