#ifndef TINY3D_H
#define TINY3D_H

// Main public header for the libtiny3d library.
// Include this file to access all core functionalities.

#include "canvas.h"
#include "math3d.h"
#include "renderer.h" // Includes lighting.h implicitly if renderer.h is well-structured
#include "lighting.h" // Explicitly include for direct access if needed, or rely on renderer.h
#include "animation.h"// Includes renderer.h for model_t

// Note on includes within headers:
// - canvas.h is self-contained or includes stdlib/stdio.
// - math3d.h is self-contained or includes math.h.
// - lighting.h includes math3d.h.
// - renderer.h includes math3d.h, canvas.h, and lighting.h.
// - animation.h includes math3d.h and renderer.h.
//
// Including them all here ensures a user of tiny3d.h gets everything.
// The individual headers should also have include guards to prevent multiple inclusions.

#endif // TINY3D_H
