#include "../include/canvas.h"
#include <stdio.h>


char canvas[HEIGHT][WIDTH];

void default_canvas() {
	for(int i  = 0; i < HEIGHT; i++){
		for (int j = 0; j < WIDTH; j++){
			canvas[i][j] = '.';
		}
	}
}

void render_canvas()
{
  for (int i = 0; i < HEIGHT; i++)
  {
    printf("%s\n", canvas[i]);
  }

}

int main()
{
  default_canvas();
  render_canvas();

  return 0;
}
