// animation.c
#include "../include/animation.h"

// Cubic Bézier curve interpolation
vec3_t bezier(vec3_t p0, vec3_t p1, vec3_t p2, vec3_t p3, float t) {
    float u = 1.0f - t;
    float b0 = u * u * u;
    float b1 = 3 * u * u * t;
    float b2 = 3 * u * t * t;
    float b3 = t * t * t;

    vec3_t result;
    result.x = b0 * p0.x + b1 * p1.x + b2 * p2.x + b3 * p3.x;
    result.y = b0 * p0.y + b1 * p1.y + b2 * p2.y + b3 * p3.y;
    result.z = b0 * p0.z + b1 * p1.z + b2 * p2.z + b3 * p3.z;

    return vec3_from_cartesian(result.x, result.y, result.z);
}
