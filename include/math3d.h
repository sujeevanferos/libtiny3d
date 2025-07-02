#ifndef MATH3D_H
#define MATH3D_H

#include <math.h>

typedef struct {
    float x, y, z;
    float r, theta, phi;
} vec3_t;

vec3_t vec3_from_cartesian (float x, float y, float z);
vec3_t vec3_from_spherical (float r, float theta, float phi);

void vec3_update_spherical (vec3_t* v);
void vec3_update_cartesian (vec3_t* v);

vec3_t vec3_normalize_fast(vec3_t v);
vec3_t vec3_slerp (vec3_t a, vec3_t b, float t);

#endif