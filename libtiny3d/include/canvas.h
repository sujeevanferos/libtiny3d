#ifndef CANVAS_H
#define CANVAS_H

#include <stdlib.h> // For size_t

// Represents a canvas for drawing.
// Pixels are stored as float values between 0.0 (black) and 1.0 (white).
typedef struct {
    int width;
    int height;
    float *pixels; // 2D array stored as a 1D array (row-major order)
    float active_viewport_radius; // For circular viewport clipping. 0 or negative means no clipping.
} canvas_t;

// Function prototypes

/**
 * @brief Sets the active circular viewport radius for clipping.
 *
 * Subsequent calls to set_pixel_f (and thus draw_line_f) will clip pixels
 * outside this circular viewport.
 *
 * @param canvas A pointer to the canvas_t.
 * @param radius The radius of the circular viewport. Set to 0 or negative to disable circular clipping.
 */
void canvas_set_circular_viewport(canvas_t* canvas, float radius);

/**
 * @brief Creates a new canvas.
 *
 * @param width The width of the canvas in pixels.
 * @param height The height of the canvas in pixels.
 * @return A pointer to the newly created canvas_t, or NULL if memory allocation fails.
 *         The caller is responsible for freeing the canvas using canvas_destroy.
 */
canvas_t* canvas_create(int width, int height);

/**
 * @brief Destroys a canvas and frees its memory.
 *
 * @param canvas A pointer to the canvas_t to destroy.
 */
void canvas_destroy(canvas_t* canvas);

/**
 * @brief Sets the brightness of a pixel using bilinear filtering for sub-pixel accuracy.
 *
 * The brightness is distributed among the 4 neighboring pixels based on the
 * fractional part of the coordinates.
 *
 * @param canvas A pointer to the canvas_t.
 * @param x The x-coordinate (can be a float).
 * @param y The y-coordinate (can be a float).
 * @param intensity The brightness value (0.0 to 1.0).
 */
void set_pixel_f(canvas_t* canvas, float x, float y, float intensity);

/**
 * @brief Draws a line on the canvas using the DDA algorithm with thickness.
 *
 * @param canvas A pointer to the canvas_t.
 * @param x0 The starting x-coordinate of the line.
 * @param y0 The starting y-coordinate of the line.
 * @param x1 The ending x-coordinate of the line.
 * @param y1 The ending y-coordinate of the line.
 * @param thickness The thickness of the line.
 * @param line_intensity The intensity (brightness, 0.0 to 1.0) of the line.
 */
void draw_line_f(canvas_t* canvas, float x0, float y0, float x1, float y1, float thickness, float line_intensity);

/**
 * @brief Clears the canvas to a specific intensity (e.g., 0.0 for black).
 *
 * @param canvas A pointer to the canvas_t.
 * @param intensity The intensity to clear the canvas to.
 */
void canvas_clear(canvas_t* canvas, float intensity);

/**
 * @brief Saves the canvas to a PGM (Portable GrayMap) file.
 *
 * @param canvas A pointer to the canvas_t.
 * @param filename The name of the file to save to.
 * @return 0 on success, -1 on error.
 */
int canvas_save_to_pgm(const canvas_t* canvas, const char* filename);


#endif // CANVAS_H
