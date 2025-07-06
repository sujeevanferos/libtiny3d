#include "math3d.h"
#include <math.h>
#include <string.h> // For memcpy
#include <stdio.h>  // For potential error messages or debugging
#include <stdint.h> // For int32_t

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define FLT_EPSILON 1.19209290E-07F // From float.h, approx

// --- Internal Helper Functions for vec3_t Synchronization ---

// Updates spherical coordinates from Cartesian coordinates
static void _vec3_update_spherical_from_cartesian(vec3_t* v) {
    v->r = sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
    if (v->r == 0.0f) {
        v->theta = 0.0f;
        v->phi = 0.0f;
    } else {
        v->theta = acosf(v->z / v->r); // Polar angle (from +Z axis)
        v->phi = atan2f(v->y, v->x);   // Azimuthal angle (in XY plane, from +X axis)
        if (v->phi < 0.0f) {
            v->phi += 2.0f * M_PI; // Ensure phi is in [0, 2*PI)
        }
    }
}

// Updates Cartesian coordinates from spherical coordinates
static void _vec3_update_cartesian_from_spherical(vec3_t* v) {
    float sin_theta = sinf(v->theta);
    v->x = v->r * sin_theta * cosf(v->phi);
    v->y = v->r * sin_theta * sinf(v->phi);
    v->z = v->r * cosf(v->theta);
}

// --- vec3_t Public Functions ---

void vec3_set_cartesian(vec3_t* v, float x, float y, float z) {
    if (!v) return;
    v->x = x;
    v->y = y;
    v->z = z;
    _vec3_update_spherical_from_cartesian(v);
}

void vec3_set_spherical(vec3_t* v, float r, float theta, float phi) {
    if (!v) return;
    v->r = r;
    v->theta = theta; // Assuming theta is polar [0, PI]
    v->phi = phi;     // Assuming phi is azimuthal [0, 2PI]
    _vec3_update_cartesian_from_spherical(v);
}

float vec3_get_x(const vec3_t* v) { return v ? v->x : 0.0f; }
float vec3_get_y(const vec3_t* v) { return v ? v->y : 0.0f; }
float vec3_get_z(const vec3_t* v) { return v ? v->z : 0.0f; }
float vec3_get_r(const vec3_t* v) { return v ? v->r : 0.0f; }
float vec3_get_theta(const vec3_t* v) { return v ? v->theta : 0.0f; }
float vec3_get_phi(const vec3_t* v) { return v ? v->phi : 0.0f; }

vec3_t vec3_create_cartesian(float x, float y, float z) {
    vec3_t v;
    vec3_set_cartesian(&v, x, y, z);
    return v;
}

vec3_t vec3_from_spherical(float r, float inclination_theta, float azimuth_phi) {
    vec3_t v;
    vec3_set_spherical(&v, r, inclination_theta, azimuth_phi);
    return v;
}

void vec3_normalize(vec3_t* v) {
    if (!v) return;
    // Re-calculate r from current Cartesian, in case they were modified directly
    // _vec3_update_spherical_from_cartesian(v); // This would re-calculate r
    // Or, just use current v->r if we trust it. For safety, recalculate:
    float current_r = sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);

    if (current_r < FLT_EPSILON) { // Check against a small epsilon to avoid division by zero
        // Cannot normalize a zero vector. Set to a default or leave as is.
        // For now, leave as zero vector. Or set x=0,y=0,z=0 explicitly.
        v->x = 0.0f; v->y = 0.0f; v->z = 0.0f;
        v->r = 0.0f; v->theta = 0.0f; v->phi = 0.0f;
        return;
    }
    v->x /= current_r;
    v->y /= current_r;
    v->z /= current_r;
    // After normalization, r is 1. Theta and phi remain the same.
    v->r = 1.0f;
    // Spherical coords (theta, phi) don't change direction, only r changes to 1.
    // If x,y,z were set manually and r was stale, then theta/phi need update.
    // _vec3_update_spherical_from_cartesian(v) would correctly set r=1 and update theta,phi.
    // For now, assuming x,y,z are primary or r was consistent.
}

// Fast inverse square root (from Quake III Arena)
// Modified to use memcpy to reduce strict-aliasing warnings.
// Assumes float is 32-bit and we want to manipulate its bits as a 32-bit integer.
static float _q_rsqrt(float number) {
    int32_t i; // Use int32_t for explicit 32-bit size, matching float
    float x2, y = 0.0f; // Initialize y
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    y = number; // y is now properly initialized before its bits are used

    // Type punning using memcpy. Copy float bits to int32_t.
    memcpy(&i, &y, sizeof(float)); // sizeof(float) should be sizeof(int32_t)

    i = 0x5f3759df - (i >> 1);  // The magic number. This works on the 32-bit integer representation.

    // Copy manipulated bits back to float.
    memcpy(&y, &i, sizeof(float)); // sizeof(float) should be sizeof(int32_t)

    y = y * (threehalfs - (x2 * y * y)); // 1st iteration (Newton's method)
    // y  = y * ( threehalfs - ( x2 * y * y ) ); // 2nd iteration (optional for more precision)
    return y;
}

void vec3_normalize_fast(vec3_t* v) {
    if (!v) return;
    float mag_sq = v->x * v->x + v->y * v->y + v->z * v->z;

    if (mag_sq < FLT_EPSILON * FLT_EPSILON || mag_sq == 0.0f) { // Check for zero or very small magnitude
        v->x = 0.0f; v->y = 0.0f; v->z = 0.0f;
        v->r = 0.0f; v->theta = 0.0f; v->phi = 0.0f;
        return;
    }

    float inv_mag = _q_rsqrt(mag_sq);
    v->x *= inv_mag;
    v->y *= inv_mag;
    v->z *= inv_mag;

    // Update spherical coordinates
    v->r = 1.0f; // By definition of normalization
    // Theta and phi remain the same directionally, but recalculate for precision if needed
    // For performance, one might assume they don't need full update if only r changes.
    // However, if x,y,z were directly manipulated, theta/phi might be stale.
    // Let's ensure they are correct for the new normalized x,y,z.
    // Theta calculation from normalized z:
    if (v->z > 1.0f) v->z = 1.0f; // Clamp due to potential precision errors
    if (v->z < -1.0f) v->z = -1.0f;
    v->theta = acosf(v->z); // r is 1, so z_norm = cos(theta)
    v->phi = atan2f(v->y, v->x); // y_norm / x_norm
    if (v->phi < 0.0f) {
        v->phi += 2.0f * M_PI;
    }
}


// Dot product of two vectors (Cartesian components)
// Note: This is a static helper. A public vec3_dot might be useful too.
// For now, it's used by vec3_slerp.
static float vec3_dot(const vec3_t* a, const vec3_t* b) {
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

// Cross product of two vectors (Cartesian components)
// Made public.
vec3_t vec3_cross(const vec3_t* a, const vec3_t* b) {
    vec3_t result;
    float x = a->y * b->z - a->z * b->y;
    float y = a->z * b->x - a->x * b->z;
    float z = a->x * b->y - a->y * b->x;
    vec3_set_cartesian(&result, x, y, z);
    return result;
}


// Spherical Linear Interpolation (SLERP)
// Assumes a and b are normalized. The result will also be normalized.
vec3_t vec3_slerp(const vec3_t* v_a, const vec3_t* v_b, float t) {
    // Clamp t to [0, 1]
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    // Dot product between the two vectors
    float dot = vec3_dot(v_a, v_b);

    // Clamp dot to [-1, 1] due to potential floating point inaccuracies
    if (dot < -1.0f) dot = -1.0f;
    if (dot > 1.0f) dot = 1.0f;

    float theta_angle = acosf(dot); // Angle between vectors

    vec3_t result;

    if (fabsf(theta_angle) < FLT_EPSILON || fabsf(dot - 1.0f) < FLT_EPSILON) {
        // If angle is very small, linear interpolation is fine (and avoids division by sin(theta_angle) ~ 0)
        // Or if vectors are identical
        result = *v_a; // Or *v_b, they are effectively the same
        return result; // Or lerp if they are not perfectly identical but close
    }

    if (fabsf(dot + 1.0f) < FLT_EPSILON) { // Vectors are opposite
        // Standard slerp is undefined here. Can pick an arbitrary orthogonal axis.
        // For simplicity, if t=0.5, result could be a zero vector or an arbitrary perpendicular.
        // A robust way is to rotate one vector by PI around an arbitrary axis perpendicular to it.
        // However, a common fallback for opposite vectors is to simply return one of them or lerp.
        // If we must slerp, and they are exactly opposite, the path is ambiguous.
        // For now, let's handle this by picking a path.
        // If t is 0.5, any vector orthogonal to v_a is a valid result.
        // This case is tricky. For now, we'll use a simple LERP and re-normalize,
        // which is not true slerp for opposite vectors.
        // A better approach for perfectly opposite vectors might involve quaternions or a more complex geometric construction.
        // For now, if they are nearly opposite, the standard formula below should still work if sin_theta is not zero.
    }


    float sin_theta = sinf(theta_angle);

    if (fabsf(sin_theta) < FLT_EPSILON) { // Vectors are collinear
        // Should have been caught by dot product check, but as a safeguard
        result = *v_a; // Default to v_a if sin_theta is zero
        if (t > 0.5f) result = *v_b;
        return result;
    }

    float scale_a = sinf((1.0f - t) * theta_angle) / sin_theta;
    float scale_b = sinf(t * theta_angle) / sin_theta;

    float res_x = scale_a * v_a->x + scale_b * v_b->x;
    float res_y = scale_a * v_a->y + scale_b * v_b->y;
    float res_z = scale_a * v_a->z + scale_b * v_b->z;
    vec3_set_cartesian(&result, res_x, res_y, res_z);

    // The result of slerp should be normalized if inputs are normalized.
    // Re-normalize to be safe due to potential floating point errors.
    vec3_normalize(&result);

    return result;
}


// --- mat4_t Functions ---

mat4_t mat4_identity() {
    mat4_t mat;
    // Uses memset to zero out and then sets diagonal elements
    memset(mat.m, 0, 16 * sizeof(float));
    mat.m[0] = 1.0f;  // Col 0, Row 0
    mat.m[5] = 1.0f;  // Col 1, Row 1
    mat.m[10] = 1.0f; // Col 2, Row 2
    mat.m[15] = 1.0f; // Col 3, Row 3
    return mat;
}

mat4_t mat4_translate(float tx, float ty, float tz) {
    mat4_t mat = mat4_identity();
    // Translation components go in the last column for column-major matrix
    mat.m[12] = tx; // Col 3, Row 0
    mat.m[13] = ty; // Col 3, Row 1
    mat.m[14] = tz; // Col 3, Row 2
    // mat.m[15] is already 1.0 from identity
    return mat;
}

mat4_t mat4_scale(float sx, float sy, float sz) {
    mat4_t mat = mat4_identity(); // Start with identity
    mat.m[0] = sx;  // Scale X on diagonal
    mat.m[5] = sy;  // Scale Y on diagonal
    mat.m[10] = sz; // Scale Z on diagonal
    // mat.m[15] remains 1.0
    return mat;
}

// Made public - remove static
mat4_t mat4_rotate_x(float angle_rad) {
    mat4_t m = mat4_identity();
    float c = cosf(angle_rad);
    float s = sinf(angle_rad);
    // Column 1
    m.m[5] = c;
    m.m[6] = s;
    // Column 2
    m.m[9] = -s;
    m.m[10] = c;
    return m;
}

// Made public - remove static
mat4_t mat4_rotate_y(float angle_rad) {
    mat4_t m = mat4_identity();
    float c = cosf(angle_rad);
    float s = sinf(angle_rad);
    // Column 0
    m.m[0] = c;
    m.m[2] = -s; // Note: some conventions might have s here. For right-handed system, rotate Y means Z maps to -X for positive angle.
    // Column 2
    m.m[8] = s;  // And X maps to Z.
    m.m[10] = c;
    return m;
}

// Made public - remove static
mat4_t mat4_rotate_z(float angle_rad) {
    mat4_t m = mat4_identity();
    float c = cosf(angle_rad);
    float s = sinf(angle_rad);
    // Column 0
    m.m[0] = c;
    m.m[1] = s;
    // Column 1
    m.m[4] = -s;
    m.m[5] = c;
    return m;
}

// Creates a rotation matrix from Euler angles (applied in Z, then Y, then X order)
// This is a common convention, e.g., R = Rx * Ry * Rz.
// If the requirement is Rx * Ry * Rz (local axes) or Rz * Ry * Rx (fixed axes), this needs care.
// The problem states "XYZ order", typically meaning fixed axes rotation: Rz, then Ry, then Rx.
// So, the combined matrix is M = M_x * M_y * M_z.
mat4_t mat4_rotate_xyz(float rx_rad, float ry_rad, float rz_rad) {
    mat4_t rot_x = mat4_rotate_x(rx_rad);
    mat4_t rot_y = mat4_rotate_y(ry_rad);
    mat4_t rot_z = mat4_rotate_z(rz_rad);

    // Combine them: M = M_x * M_y * M_z
    // Order of multiplication matters. If it's local rotations, order might be different.
    // Assuming fixed axis rotations: first Z, then Y, then X.
    // So, point p is transformed by p' = Rx * Ry * Rz * p
    mat4_t ry_rz = mat4_multiply(&rot_y, &rot_z);
    mat4_t rx_ry_rz = mat4_multiply(&rot_x, &ry_rz);

    return rx_ry_rz;
}


mat4_t mat4_multiply(const mat4_t* a, const mat4_t* b) {
    mat4_t res;
    for (int i = 0; i < 4; ++i) { // column of result
        for (int j = 0; j < 4; ++j) { // row of result
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) {
                // res.m[col][row] = sum(a.m[k][row] * b.m[col][k]) for row-major
                // For column-major a.m[row + col*4]:
                // res.m[i*4+j] = sum(a.m[k*4+j] * b.m[i*4+k])
                // Or, more standard: C_ij = A_ik * B_kj (sum over k)
                // C[col][row] = A[col][k_row] * B[k_col][row]
                // In 1D array (column-major):
                // C[col*4 + row] = sum over k ( A[k*4 + row] * B[col*4 + k] ) -- this is if A is indexed by k as column
                // Let's use standard notation: C_ij = sum_k A_ik * B_kj
                // If m[c*4+r]: C(i,j) is m[i*4+j] where i is col, j is row
                // A(k,j) is a->m[k*4+j]
                // B(i,k) is b->m[i*4+k]
                // So, res.m[i*4+j] = sum_k ( a->m[k*4+j] * b->m[i*4+k] ) -- This is wrong.

                // Correct for Column Major C=A*B:
                // C_rc = sum_k A_rk * B_kc (if thinking row,col indices)
                // In memory M[col*4 + row]:
                // M_res[c*4 + r] = sum_k ( M_a[k*4 + r] * M_b[c*4 + k] )
                sum += a->m[k * 4 + j] * b->m[i * 4 + k];
            }
            res.m[i * 4 + j] = sum;
        }
    }
    return res;
}


// Transforms a vec3_t point by a mat4_t (assumes w=1 for point).
vec3_t mat4_transform_point(const mat4_t* mat, const vec3_t* p) {
    vec3_t res;
    float x = p->x;
    float y = p->y;
    float z = p->z;
    float w = 1.0f; // Assume w=1 for points

    // Using column-major matrix m[col*4 + row]
    res.x = mat->m[0]*x + mat->m[4]*y + mat->m[8]*z  + mat->m[12]*w;
    res.y = mat->m[1]*x + mat->m[5]*y + mat->m[9]*z  + mat->m[13]*w;
    res.z = mat->m[2]*x + mat->m[6]*y + mat->m[10]*z + mat->m[14]*w;
    float res_w = mat->m[3]*x + mat->m[7]*y + mat->m[11]*z + mat->m[15]*w;

    // Perspective divide if w is not 0 or 1
    if (fabsf(res_w) > FLT_EPSILON && fabsf(res_w - 1.0f) > FLT_EPSILON) {
        res.x /= res_w;
        res.y /= res_w;
        res.z /= res_w;
    }
    _vec3_update_spherical_from_cartesian(&res); // Update spherical part
    return res;
}

// Transforms a vec3_t vector by a mat4_t (assumes w=0 for vector/direction).
vec3_t mat4_transform_vector(const mat4_t* mat, const vec3_t* v_vec) {
    vec3_t res;
    float x = v_vec->x;
    float y = v_vec->y;
    float z = v_vec->z;
    // w = 0 for vectors (directions are not affected by translation)
    res.x = mat->m[0]*x + mat->m[4]*y + mat->m[8]*z;  // + mat->m[12]*0
    res.y = mat->m[1]*x + mat->m[5]*y + mat->m[9]*z;  // + mat->m[13]*0
    res.z = mat->m[2]*x + mat->m[6]*y + mat->m[10]*z; // + mat->m[14]*0
    // w component for a transformed vector is mat->m[3]*x + mat->m[7]*y + mat->m[11]*z + mat->m[15]*0
    // This should remain 0 if the matrix is affine. If it's a projection matrix, this might not make sense.
    // For now, we assume it's for transforming directions (like normals) with TRS matrices.
    _vec3_update_spherical_from_cartesian(&res);
    return res;
}


// Creates an asymmetric perspective projection frustum matrix (OpenGL style).
// (left, right, bottom, top, nearVal, farVal)
// nearVal and farVal are positive distances from the camera.
mat4_t mat4_frustum_asymmetric(float l, float r, float b, float t, float n, float f) {
    mat4_t mat;
    memset(mat.m, 0, 16 * sizeof(float));

    // Column 0
    mat.m[0] = (2.0f * n) / (r - l);
    // mat.m[1] = 0;
    // mat.m[2] = 0;
    // mat.m[3] = 0;

    // Column 1
    // mat.m[4] = 0;
    mat.m[5] = (2.0f * n) / (t - b);
    // mat.m[6] = 0;
    // mat.m[7] = 0;

    // Column 2
    mat.m[8] = (r + l) / (r - l);
    mat.m[9] = (t + b) / (t - b);
    mat.m[10] = -(f + n) / (f - n);
    mat.m[11] = -1.0f; // Projects z to -w (depth direction)

    // Column 3
    // mat.m[12] = 0;
    // mat.m[13] = 0;
    mat.m[14] = -(2.0f * f * n) / (f - n);
    // mat.m[15] = 0; // w component will be -z_camera_space

    return mat;
}

// Creates a symmetric perspective projection matrix.
// fovy: field of view in Y direction, in radians
// aspect: aspect ratio (width / height)
// n: near clipping plane (positive)
// f: far clipping plane (positive)
mat4_t mat4_perspective(float fovy_rad, float aspect, float n, float f) {
    float tan_half_fovy = tanf(fovy_rad / 2.0f);

    float t = n * tan_half_fovy;       // top
    float b = -t;                      // bottom
    float r = t * aspect;              // right
    float l = -r;                      // left

    return mat4_frustum_asymmetric(l, r, b, t, n, f);
}


// --- Quaternion Functions ---

// Normalize a quaternion
static void quat_normalize(quat_t* q) {
    float mag = sqrtf(q->x*q->x + q->y*q->y + q->z*q->z + q->w*q->w);
    if (mag > FLT_EPSILON) {
        q->x /= mag;
        q->y /= mag;
        q->z /= mag;
        q->w /= mag;
    } else { // Should not happen with valid rotations
        q->x = 0; q->y = 0; q->z = 0; q->w = 1; // Identity
    }
}

quat_t quat_from_axis_angle(vec3_t axis, float angle_rad) {
    quat_t q;
    // Ensure axis is normalized (though vec3_normalize could be called by user)
    // vec3_normalize(&axis); // Or assume it's normalized

    float half_angle = angle_rad * 0.5f;
    float s = sinf(half_angle);

    q.x = axis.x * s;
    q.y = axis.y * s;
    q.z = axis.z * s;
    q.w = cosf(half_angle);

    // Quaternions representing rotations should be unit quaternions.
    // Normalization might be needed if axis wasn't unit or due to precision.
    // For simplicity, assume axis is normalized and calculations are precise enough.
    // quat_normalize(&q); // Optional, for robustness
    return q;
}

mat4_t quat_to_mat4(quat_t q) {
    mat4_t m = mat4_identity();

    // Ensure quaternion is normalized for accurate conversion
    // quat_normalize(&q); // Assuming q is already normalized

    float qx = q.x, qy = q.y, qz = q.z, qw = q.w;
    float qx2 = qx*qx, qy2 = qy*qy, qz2 = qz*qz;
    // float qw2 = qw*qw; // Not directly used in this form

    // Diagonal elements
    m.m[0]  = 1.0f - 2.0f*qy2 - 2.0f*qz2; // Col 0, Row 0
    m.m[5]  = 1.0f - 2.0f*qx2 - 2.0f*qz2; // Col 1, Row 1
    m.m[10] = 1.0f - 2.0f*qx2 - 2.0f*qy2; // Col 2, Row 2

    // Off-diagonal elements
    // Col 0
    m.m[1]  = 2.0f*qx*qy + 2.0f*qz*qw;      // Row 1
    m.m[2]  = 2.0f*qx*qz - 2.0f*qy*qw;      // Row 2

    // Col 1
    m.m[4]  = 2.0f*qx*qy - 2.0f*qz*qw;      // Row 0
    m.m[6]  = 2.0f*qy*qz + 2.0f*qx*qw;      // Row 2

    // Col 2
    m.m[8]  = 2.0f*qx*qz + 2.0f*qy*qw;      // Row 0
    m.m[9]  = 2.0f*qy*qz - 2.0f*qx*qw;      // Row 1

    // m.m[15] = 1.0f; // from identity
    return m;
}

// Dot product of two quaternions
static float quat_dot(quat_t q1, quat_t q2) {
    return q1.x*q2.x + q1.y*q2.y + q1.z*q2.z + q1.w*q2.w;
}

quat_t quat_slerp(quat_t qa, quat_t qb, float t) {
    // Ensure t is clamped between 0 and 1
    t = fmaxf(0.0f, fminf(1.0f, t));

    float dot = quat_dot(qa, qb);
    quat_t q_result;

    // If the dot product is negative, the quaternions are more than 90 degrees apart.
    // Slerp won't take the shorter path. So, we invert one quaternion.
    quat_t qb_prime = qb;
    if (dot < 0.0f) {
        qb_prime.x = -qb.x;
        qb_prime.y = -qb.y;
        qb_prime.z = -qb.z;
        qb_prime.w = -qb.w;
        dot = -dot;
    }

    // If the quaternions are too close, just linearly interpolate and normalize.
    const float DOT_THRESHOLD = 0.9995f; // Threshold for considering quaternions "close"
    if (dot > DOT_THRESHOLD) {
        q_result.x = qa.x + t * (qb_prime.x - qa.x);
        q_result.y = qa.y + t * (qb_prime.y - qa.y);
        q_result.z = qa.z + t * (qb_prime.z - qa.z);
        q_result.w = qa.w + t * (qb_prime.w - qa.w);
    } else {
        // Standard slerp
        float theta_0 = acosf(dot);          // angle between input quaternions
        float theta = theta_0 * t;           // angle from qa to result
        float sin_theta = sinf(theta);
        float sin_theta_0 = sinf(theta_0);

        float scale_qa = cosf(theta) - dot * sin_theta / sin_theta_0; // Equivalent to sinf(theta_0 - theta) / sin_theta_0
        float scale_qb = sin_theta / sin_theta_0;

        q_result.x = scale_qa * qa.x + scale_qb * qb_prime.x;
        q_result.y = scale_qa * qa.y + scale_qb * qb_prime.y;
        q_result.z = scale_qa * qa.z + scale_qb * qb_prime.z;
        q_result.w = scale_qa * qa.w + scale_qb * qb_prime.w;
    }

    quat_normalize(&q_result);
    return q_result;
}
