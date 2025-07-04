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
        .x = w1 * a.x + w2 * b.x, 
        .y = w1 * a.y + w2 * b.y,
        .z = w1 * a.z + w2 * b.z,
        .r = 0.0f, .theta = 0.0f, .phi = 0.0f
    };

    vec3_update_spherical(&result);
    return result;
}

mat4_t mat4_identity(){
    mat4_t m = {0};
    m.m[0] = m.m[5] = m.m[10] = m.m[15] = 1.0f;
    return m;
}

mat4_t mat4_translate(float tx, float ty, float tz){
    mat4_t m = mat4_identity();
    m.m[12] = tx;
    m.m[13] = ty;
    m.m[14] = tz;

    return m;
}

mat4_t mat4_scale(float sx, float sy, float sz){
    mat4_t m = {0};
    m.m[0] = sx;
    m.m[5] = sy;
    m.m[10] = sz;
    m.m[15] = 1.0f;

    return m;
}

mat4_t mat4_rotate_xyz(float rx, float ry, float rz){
    float cx = cosf(rx), sx = sinf(rx);
    float cy = cosf(ry), sy = sinf(ry);
    float cz = cosf(rz), sz = sinf(rz);

    mat4_t rotX = mat4_identity();
    rotX.m[5] = cx; rotX.m[6] = -sx;
    rotX.m[9] = sx; rotX.m[10] = cx;

    mat4_t rotY = mat4_identity();
    rotY.m[0] = cy; rotY.m[2] = sy;
    rotY.m[8] = -sy; rotY.m[10] = cy;

    mat4_t rotZ = mat4_identity();
    rotZ.m[0] = cz; rotZ.m[1] = -sz;
    rotZ.m[4] = sz; rotZ.m[5] = cz;
    
    mat4_t temp = mat4_mul(rotZ, rotY);
    return mat4_mul(temp, rotX);
}

vec3_t mat4_mul_vec3(mat4_t m, vec3_t v){
    float x = v.x, y = v.y, z = v.z, w = 1.0f;

    float tx = m.m[0]*x + m.m[4]*y + m.m[8]*z + m.m[12]*w;
    float ty = m.m[1]*x + m.m[5]*y + m.m[9]*z + m.m[13]*w;
    float tz = m.m[2]*x + m.m[6]*y + m.m[10]*z + m.m[14]*w;
    float tw = m.m[3]*x + m.m[7]*y + m.m[11]*z + m.m[15]*w;

    if (fabs(tw) > 1e-6f){
        tx /= tw; 
        ty /= tw; 
        tz /= tw;
    }

    return vec3_from_cartesian(tx, ty, tz);
}

mat4_t mat4_mul(mat4_t a, mat4_t b){
    mat4_t r = {0};
    for (int row = 0; row < 4; ++row){
        for (int col = 0; col < 4; ++col){
            for (int i = 0; i < 4; ++i){
                r.m[col + row*4] += a.m[i + row*4] * b.m[col + i*4];
            }
        }
    }

    return r;
}

mat4_t mat4_frustum_asymmetric(float l, float r, float b, float t, float n, float f) {
    mat4_t m = {0};

    m.m[0] = (2.0f * n) / (r - l);
    m.m[5] = (2.0f * n) / (t - b);
    m.m[8] = (r + l) / (r - l);
    m.m[9] = (t + b) / (t - b);
    m.m[10] = -(f + n) / (f - n);
    m.m[11] = -1.0f;
    m.m[14] = -(2.0f * f * n) / (f - n);

    return m;
}

