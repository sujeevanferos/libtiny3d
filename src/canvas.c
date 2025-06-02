#include <stdio.h>

#define HEIGHT 20
#define WIDTH 30

char canvas[HEIGHT][WIDTH]

void default_canvas() {
	for(int i  = 0; i < HEIGHT; i++){
		for (int j = 0; j < WIDTH; j++){
			canvas[i][j] = '. ';
		}
	}
}

void render_canvas()
{

}
