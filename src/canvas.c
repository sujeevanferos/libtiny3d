#include "../include/canvas.h"
#include <stdio.h>

char canvas[HEIGHT][WIDTH+1];

void clear_canvas()
{
  for(int j = 0; j < HEIGHT; j++)
  {
    for(int i = 0; i < WIDTH; i++)
    {
      canvas[j][i] = '.';
    }
    canvas[j][WIDTH] = '\0';
  }
}

void render_canvas()
{
  for(int y = 0; y < HEIGHT; y++)
  {
    printf("%s", canvas[y]);
  }
}

void set_pixel(int y, int x, char ch)
{
  if(x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
  {
    canvas[y][x] = ch;
  }
}


