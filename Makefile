CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -O2
SRC = src/canvas.c demo/main.c
OUT = demo/demo

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) -lm

clean:
	rm -f $(OUT) canvas_output.pgm