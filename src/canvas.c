#include "../include/canvas.h"
#include <stdio.h>  // For FILE operations in canvas_save_to_pgm
#include <stdlib.h> // For malloc, free
#include <string.h> // For memset
#include <math.h>   // For floor, ceil, fmax, fmin, sqrtf
#include <float.h>  // For FLT_EPSILON

// Static helper function for circular viewport clipping
// Moved from renderer.c to be used by set_pixel_f directly.
static int _canvas_is_pixel_in_circular_viewport(const canvas_t* canvas, int px, int py) {
    if (!canvas || canvas->active_viewport_radius <= 0.0f) {
        return 1; // No clipping or invalid canvas
    }

    float radius = canvas->active_viewport_radius;
    // If active_viewport_radius was set to a "default" sentinel (e.g. -1), calculate actual default.
    // For now, assume active_viewport_radius is the actual radius if > 0.
    // A more robust way would be if render_wireframe calculates the actual radius and sets it.

    float center_x = canvas->width / 2.0f;
    float center_y = canvas->height / 2.0f;

    // Using integer pixel coordinates for check against center
    float dist_sq = ( (float)px - center_x ) * ( (float)px - center_x ) +
                    ( (float)py - center_y ) * ( (float)py - center_y );

    return dist_sq <= (radius * radius);
}


canvas_t* canvas_create(int width, int height) {
    if (width <= 0 || height <= 0) {
        fprintf(stderr, "Error: Canvas dimensions must be positive.\n");
        return NULL;
    }

    canvas_t* canvas = (canvas_t*)malloc(sizeof(canvas_t));
    if (!canvas) {
        fprintf(stderr, "Error: Failed to allocate memory for canvas_t struct.\n");
        return NULL;
    }

    canvas->width = width;
    canvas->height = height;
    canvas->pixels = (float*)malloc(width * height * sizeof(float));
    canvas->active_viewport_radius = 0.0f; // Initialize to no clipping

    if (!canvas->pixels) {
        fprintf(stderr, "Error: Failed to allocate memory for canvas pixels.\n");
        free(canvas);
        return NULL;
    }

    // Initialize pixels to 0.0 (black)
    memset(canvas->pixels, 0, width * height * sizeof(float));

    return canvas;
}

void canvas_destroy(canvas_t* canvas) {
    if (canvas) {
        free(canvas->pixels);
        free(canvas);
    }
}

void canvas_set_circular_viewport(canvas_t* canvas, float radius) {
    if (canvas) {
        canvas->active_viewport_radius = radius;
    }
}

void canvas_clear(canvas_t* canvas, float intensity) {
    if (!canvas || !canvas->pixels) {
        return;
    }
    for (int i = 0; i < canvas->width * canvas->height; ++i) {
        canvas->pixels[i] = intensity;
    }
}

void set_pixel_f(canvas_t* canvas, float x, float y, float intensity) {
    if (!canvas || !canvas->pixels) {
        return;
    }

    // Clamp intensity to [0, 1]
    intensity = fmaxf(0.0f, fminf(1.0f, intensity));

    // Integer parts of the coordinates
    int x_int = (int)floorf(x);
    int y_int = (int)floorf(y);

    // Fractional parts of the coordinates
    float fx = x - x_int;
    float fy = y - y_int;

    // Iterate over the 2x2 pixel neighborhood
    for (int j = 0; j <= 1; ++j) { // y-offset (0 or 1)
        for (int i = 0; i <= 1; ++i) { // x-offset (0 or 1)
            int current_x = x_int + i;
            int current_y = y_int + j;

            // Check bounds
            if (current_x >= 0 && current_x < canvas->width &&
                current_y >= 0 && current_y < canvas->height) {

                // Perform circular viewport clipping for the *center* of the target pixel block
                if (!_canvas_is_pixel_in_circular_viewport(canvas, current_x, current_y)) {
                    continue; // This pixel is outside the circular viewport
                }

                // Calculate bilinear weights
                float weight_x = (i == 0) ? (1.0f - fx) : fx;
                float weight_y = (j == 0) ? (1.0f - fy) : fy;
                float weight = weight_x * weight_y;

                // Update pixel intensity (additive blending for now, can be changed)
                // The problem description "spreads the brightness" implies accumulation.
                int pixel_index = current_y * canvas->width + current_x;
                canvas->pixels[pixel_index] += intensity * weight;
                // Clamp the accumulated intensity to [0, 1]
                canvas->pixels[pixel_index] = fmaxf(0.0f, fminf(1.0f, canvas->pixels[pixel_index]));
            }
        }
    }
}

// Implementation of draw_line_f using DDA algorithm and thickness
void draw_line_f(canvas_t* canvas, float x0, float y0, float x1, float y1, float thickness, float line_intensity) {
    if (!canvas || !canvas->pixels) {
        return;
    }

    // Clamp line_intensity
    float clamped_intensity = fmaxf(0.0f, fminf(1.0f, line_intensity));
    if (clamped_intensity < FLT_EPSILON) { // If intensity is effectively zero, don't draw
        return;
    }

    float dx = x1 - x0;
    float dy = y1 - y0;

    int steps;
    if (fabsf(dx) > fabsf(dy)) {
        steps = (int)fabsf(dx);
    } else {
        steps = (int)fabsf(dy);
    }

    if (steps == 0) { // Single point
        // Draw a "thick point" which is like a small disc/square
        float half_thick = thickness / 2.0f;
        for (float ty = -half_thick; ty <= half_thick; ty += 0.5f) { // Iterate with sub-pixel steps
            for (float tx = -half_thick; tx <= half_thick; tx += 0.5f) {
                if (tx*tx + ty*ty <= half_thick*half_thick) { // Circular brush
                     set_pixel_f(canvas, x0 + tx, y0 + ty, clamped_intensity);
                }
            }
        }
        return;
    }

    float x_increment = dx / (float)steps;
    float y_increment = dy / (float)steps;

    float x = x0;
    float y = y0;

    float half_thick = fmaxf(0.5f, thickness / 2.0f); // Ensure minimum thickness for visibility

    for (int i = 0; i <= steps; ++i) {
        // For each point on the DDA line, draw a "brush" for thickness
        // A simple square brush for performance, using set_pixel_f for smoothness
        for (float brush_y = -half_thick; brush_y <= half_thick; brush_y += 0.5f) { // Iterate finer for smoother thickness
            for (float brush_x = -half_thick; brush_x <= half_thick; brush_x += 0.5f) {
                 // Optional: circular brush shape condition: if (brush_x*brush_x + brush_y*brush_y <= half_thick*half_thick)
                 set_pixel_f(canvas, x + brush_x, y + brush_y, clamped_intensity);
            }
        }
        x += x_increment;
        y += y_increment;
    }
}

// Function to save canvas to PGM - useful for debugging and demos
int canvas_save_to_pgm(const canvas_t* canvas, const char* filename) {
    if (!canvas || !canvas->pixels) {
        fprintf(stderr, "Error: Cannot save NULL canvas.\n");
        return -1;
    }
    if (!filename) {
        fprintf(stderr, "Error: Filename cannot be NULL.\n");
        return -1;
    }

    FILE* fp = fopen(filename, "wb"); // "wb" for binary PGM
    if (!fp) {
        perror("Error opening file for PGM export");
        return -1;
    }

    // PGM header:
    // P5 (binary grayscale)
    // width height
    // max_val (255 for 8-bit)
    fprintf(fp, "P5\n%d %d\n255\n", canvas->width, canvas->height);

    // Write pixel data
    for (int y = 0; y < canvas->height; ++y) {
        for (int x = 0; x < canvas->width; ++x) {
            int pixel_index = y * canvas->width + x;
            float intensity = canvas->pixels[pixel_index];
            // Clamp and scale to 0-255
            unsigned char val = (unsigned char)(fmaxf(0.0f, fminf(1.0f, intensity)) * 255.0f);
            fwrite(&val, sizeof(unsigned char), 1, fp);
        }
    }

    fclose(fp);
    return 0;
}
