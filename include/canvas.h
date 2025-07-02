#ifndef CANVAS_H
#define CANVAS_H

typedef struct {
    int height;
    int width;
    // creating a 2D array
    float **pixels;
} canvas_t;

canvas_t* create_canvas (int width, int height);
void destroy_canvas(canvas_t* canvas);

void set_pixel_f(canvas_t* canvas, float x, float y, float intensity);
void draw_line_f(canvas_t* canvas, float x0, float y0, float x1, float y1, float thickness);

#endif