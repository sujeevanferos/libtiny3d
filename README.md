# libtiny3d - 3D Software Renderer Library

`libtiny3d` is a lightweight 3D software renderer written in C from scratch, without relying on hardware acceleration libraries like OpenGL or DirectX. This project is an exercise in understanding and implementing the foundational mathematics and algorithms behind 3D graphics.

## Project Overview

This library implements a complete software rendering pipeline capable of transforming 3D geometric data into 2D images. Key features include:

-   **Canvas System**: A floating-point canvas for drawing anti-aliased lines and pixels using bilinear filtering.
-   **3D Math Library**: Custom vector (`vec3_t`) and matrix (`mat4_t`) operations, including transformations (translate, scale, rotate), quaternion rotations, SLERP, and perspective projection.
-   **Rendering Pipeline**: Functions to project 3D vertices to screen space, sort lines by depth, and render wireframe models.
-   **Lighting**: Lambertian lighting model for edges based on their direction relative to light sources.
-   **Animation**: Bézier curve-based animation for object movement.

## Current Status

The library implements the core features outlined above. The main demo (`demo/main.c`) showcases:
-   Rendering of multiple lit wireframe soccer ball models (loaded from internal OBJ data).
-   Animation of these models along Bézier paths.
-   Output as a sequence of PGM image frames to the `build/` directory, which can be converted into a video.

The soccer ball geometry is loaded from an embedded OBJ data string within the `generate_soccer_ball()` function.

## Task 1 - Create a clock face shape
To get the clock face shape:
```bash
make clean
make run_test_task1_clock
convert build/task1_clock_output.pgm tests/visual_tests/clock_shape.png
```
The shape will be shown in the `tests/visual_tests` folder as `clock_output.png`

## Task 2 - Create a cube in `test_math.c`
To get the cube model
```bash
make clean
make run_test_math
convert build/task2_math_cube_output.pgm tests/visual_tests/cube.png
```
The `.png` format of the output is stored in `tests/visual_tests/cube.png`

## Task 3 - Rotating Soccer Ball shape clipped to circular paths
```bash
make clean
make run_test_task3_soccer
ffmpeg -framerate 30 -i build/soccer_%04d.pgm -vf scale=512:512 tests/visual_tests/rotating_soccer.mp4
```
The output of the task is saved at `tests/visual_tests/rotating_soccer_ball_clipped_to_circular_path.mp4`

## Task 4 - Multiple animated objects
The shape will be shown in the `tests/visual_tests/multiple_objects_animated.mp4` path

To build and run the main demo:
```bash
make clean
make run_demo
```
This will generate `frame_xxxx.pgm` files in the `build/` directory. To get the `.mp4` version of the `Roatating Soccer Ball` model
```bash
ffmpeg -framerate 30 -i build/frame_%04d.pgm -vf scale=512:512 tests/visual_tests/multiple_objects_animated.mp4
```
All images and video outputs can be found in `tests/visual_tests`

## Project Structure

-   `include/`: Header files for the library.
-   `src/`: Source code for the library.
-   `demo/`: Source code for the main demo program.
-   `tests/`: Unit tests and visual tests.
-   `build/`: Output directory for compiled library, executables, and demo frames (gitignored).
-   `documentation/`: Project report.
-   `Makefile`: Build script for the project.
-   `README.md`: This file.

## Dependencies
- A C compiler (e.g., GCC, Clang)
- `make`
- Standard C libraries (math.h, stdio.h, stdlib.h, string.h, float.h)
- `ffmpeg`
