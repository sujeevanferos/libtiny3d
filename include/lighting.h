// lighting.h
#ifndef LIGHTING_H
#define LIGHTING_H

#include "math3d.h"

// Basic Lambert lighting calculation
float compute_lambert_intensity(vec3_t edge_dir, vec3_t light_dir);

#endif
