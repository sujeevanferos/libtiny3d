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
-   Rendering of multiple lit wireframe cube models.
-   Animation of these models along Bézier paths.
-   Output as a sequence of PGM image frames to the `build/` directory, which can be converted into a video.

The "soccer ball" model generation currently produces a cube as a placeholder due to the complexity of hardcoding true truncated icosahedron geometry.

## Building the Demo

To build and run the main demo:
```bash
make clean
make
./build/demo
```
This will generate `frame_xxxx.pgm` files in the `build/` directory.

To run the math tests:
```bash
make test_math # or make run_test_math to build and run
```

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
