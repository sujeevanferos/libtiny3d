#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../include/canvas.h"


canvas_t* create_canvas (int width, int height){
    canvas_t* canvas = (canvas_t*)malloc(sizeof(canvas_t));
    canvas->width = width;
    canvas->height = height;

    canvas->pixels = (float**)malloc(height * sizeof(float*));
    for(int y = 0; y < height; y++){
        canvas->pixels[y] = (float*)calloc(width, sizeof(float));
    }
    return canvas;
}

void destroy_canvas (canvas_t* canvas){
    for (int y = 0; y < canvas->height; y++){
        free(canvas->pixels[y]);
    }

    free(canvas->pixels);
    free(canvas);
}

void set_pixel_f (canvas_t* canvas, float x, float y, float intensity){
    int x0 = (int)floorf(x);
    int y0 = (int)floorf(y);
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    float dx = x - x0;
    float dy = y - y0;

    float w00 = (1 - dx) * (1 - dy);
    float w10 = dx * (1 - dy);
    float w01 = (1 - dx) * dy;
    float w11 = dx * dy;

    if (x0 >= 0 && y0 >= 0 && x0 < canvas->width && y0 < canvas->height)
        canvas->pixels[y0][x0] += intensity * w00;

    if (x1 >= 0 && y0 >= 0 && x1 < canvas->width && y0 < canvas->height)
        canvas->pixels[y0][x1] += intensity * w10;

    if (x0 >= 0 && y1 >= 0 && x0 < canvas->width && y1 < canvas->height)
        canvas->pixels[y1][x0] += intensity * w01;

    if (x1 >= 0 && y1 >= 0 && x1 < canvas->width && y1 < canvas->height)
        canvas->pixels[y1][x1] += intensity * w11;

}

void draw_line_f (canvas_t* canvas, float x0, float y0, float x1, float y1, float thickness){
    float dx = x1 - x0;
    float dy = y1 - y0;

    float len = sqrtf(dx * dx + dy * dy);
    int steps = (int)ceilf(len * 2);

    float step_x = dx/steps;
    float step_y = dy/steps;

    float nx = -dy/len;
    float ny = dx/len;

    for (int i = 0; i < steps; i++){
        float x = x0 + i * step_x;
        float y = y0 + i * step_y;

        for (float t = -thickness/2.0f; t <= thickness/2.0f; t += 0.5f){
            float px = x + t * nx;
            float py = y + t * ny;
            set_pixel_f(canvas, px, py, 1.0f);
        }
    }

}