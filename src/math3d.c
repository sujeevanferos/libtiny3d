#include "../include/math3d.h"
#include <math.h>

vec3_t vec3_from_cartesian (float x, float y, float z){
    vec3_t v = {x, y, z, 0.0f, 0.0f, 0.0f};
    vec3_update_spherical(&v);
    return v;
}

vec3_t vec3_from_spherical (float r, float theta, float phi) {
    vec3_t v = {
        r * sinf(phi) * cosf(theta),
        r * sinf(phi) * sinf(theta),
        r * cosf(phi),
        r, theta, phi
    };

    return v;
}

void vec3_update_spherical (vec3_t* v){
    v->r = sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
    v->theta = atan2f(v->y, v->x);
    v->phi = acosf(v->z / (v->r + 1e-6));
}

void vec3_update_cartesian(vec3_t* v){
    v->x = v->r * sinf(v->phi) * cosf(v->theta);
    v->y = v->r * sinf(v->phi) * sinf(v->theta);
    v->z = v->r * cosf(v->phi);

}

vec3_t vec3_normalize_fast(vec3_t v){
    float inv_len = 1.0f / sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    v.x *= inv_len;
    v.y *= inv_len;
    v.z *= inv_len;

    vec3_update_spherical(&v);
    return v;
}

vec3_t vec3_slerp(vec3_t a, vec3_t b, float t) {
    float dot = a.x * b.x + a.y * b.y + a.z * b.z;
    float theta = acosf(fmaxf(fminf(dot, 1.0f), -1.0f));
    float sin_theta = sinf(theta);

    if (sin_theta < 1e-5) return a;

    float w1 = sinf((1-t) * theta) / sin_theta;
    float w2 = sinf(t * theta) / sin_theta;

    vec3_t result = {
        w1 * a.x + w2 * b.x, 
        w1 * a.y + w2 * b.y,
        w1 * a.z + w2 * b.z,
        0.0f, 0.0f, 0.0f
    };

    vec3_update_spherical(&result);
    return result;
}