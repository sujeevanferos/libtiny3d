#include <stdio.h>
#include <math.h>
#include "../include/canvas.h"

#define WIDTH 512
#define HEIGHT 512

int main(){
    canvas_t* canvas = create_canvas(WIDTH, HEIGHT);
    float cx = WIDTH / 2.0f;
    float cy = HEIGHT / 2.0f;
    float radius = 200.0f;

    for (int angle = 0; angle < 360; angle += 15){
        float rad = angle * M_PI / 180.0f;
        float x = cx + radius * cosf(rad);
        float y = cy + radius * sinf(rad);

        draw_line_f(canvas, cx, cy, x, y, 1.5f);
    }

    FILE *f = fopen("canvas_output.pgm", "wb");
    fprintf(f, "P5\n%d %d\n255\n", WIDTH, HEIGHT);

    for(int y = 0; y < HEIGHT; y++){
        for (int x = 0; x < WIDTH; x++){
            unsigned char val = (unsigned char)(fminf(canvas->pixels[y][x], 1.0f) * 255.0f);
            fwrite(&val, sizeof(unsigned char), 1, f);
        }
    }


    fclose(f);

    destroy_canvas(canvas);

    return 0;
}