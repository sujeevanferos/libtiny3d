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

typedef struct {
    float m[16];
} mat4_t;

mat4_t mat4_identity(void);
mat4_t mat4_translate(float tx, float ty, float tz);
mat4_t mat4_scale(float sx, float sy, float sz);
mat4_t mat4_rotate_xyz(float rx, float ry, float rz);
vec3_t mat4_mul_vec3(mat4_t m, vec3_t v);
mat4_t mat4_mul(mat4_t a, mat4_t b);
mat4_t mat4_frustum_asymmetric(float l, float r, float b, float t, float n, float f);



#endif