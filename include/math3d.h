#ifndef MATH3D_H
#define MATH3D_H

#include <math.h> // For sqrtf, atan2f, acosf, sinf, cosf

// Forward declaration for mat4_t if used in vec3_t functions, though not directly here
// typedef struct mat4_t mat4_t;

// Structure for a 3D vector
// It stores both Cartesian and Spherical coordinates.
// Setter functions should be used to maintain synchronization.
typedef struct {
    // Cartesian coordinates
    float x, y, z;
    // Spherical coordinates (ISO convention: r, theta, phi)
    // r: radius (distance from origin)
    // theta: inclination or polar angle (angle from positive z-axis, 0 to PI)
    // phi: azimuth angle (angle from positive x-axis in xy-plane, 0 to 2*PI)
    float r, theta, phi;

    // Internal flag to denote which system was last updated
    // 0 for Cartesian, 1 for Spherical. This can help in lazy updates.
    // However, for simplicity in this pass, setter functions will update both.
} vec3_t;

// --- vec3_t Functions ---

// Setters that update both Cartesian and Spherical representations
void vec3_set_cartesian(vec3_t* v, float x, float y, float z);
void vec3_set_spherical(vec3_t* v, float r, float theta, float phi); // theta (polar), phi (azimuth)

// Getters (could just access members, but functions allow for future lazy updates)
float vec3_get_x(const vec3_t* v);
float vec3_get_y(const vec3_t* v);
float vec3_get_z(const vec3_t* v);
float vec3_get_r(const vec3_t* v);
float vec3_get_theta(const vec3_t* v); // Polar angle
float vec3_get_phi(const vec3_t* v);   // Azimuthal angle

// Creates a new vec3_t from Cartesian coordinates. Spherical coords are calculated.
vec3_t vec3_create_cartesian(float x, float y, float z);

// Creates a new vec3_t from Spherical coordinates (r, theta in [0,PI], phi in [0,2PI]). Cartesian coords are calculated.
// This is effectively the 'vec3_from_spherical' from the requirements.
vec3_t vec3_from_spherical(float r, float inclination_theta, float azimuth_phi);

// Normalizes the vector to length 1.
// Modifies the input vector v.
void vec3_normalize(vec3_t* v);

// Normalizes the vector to length 1 using fast inverse square root.
// Modifies the input vector v.
void vec3_normalize_fast(vec3_t* v);

// Calculates the cross product of two vectors (a x b).
// Returns the resulting vector.
vec3_t vec3_cross(const vec3_t* a, const vec3_t* b);

// Spherical linear interpolation between two vectors.
// a and b should be normalized vectors (directions).
// t is the interpolation factor (0.0 to 1.0).
// Returns the interpolated vector (also normalized).
vec3_t vec3_slerp(const vec3_t* a, const vec3_t* b, float t);

// --- mat4_t Structure and Operations ---

// 4x4 Matrix (Column-Major Layout internally as specified)
// m[column][row]
// Stored as a 1D array: m[0..3] = col0, m[4..7] = col1, etc.
// m[0]=m00, m[1]=m10, m[2]=m20, m[3]=m30 (col 0)
// m[4]=m01, m[5]=m11, m[6]=m21, m[7]=m31 (col 1)
// ...
typedef struct {
    float m[16];
} mat4_t;

// Creates an identity matrix.
mat4_t mat4_identity();

// Creates a translation matrix.
mat4_t mat4_translate(float tx, float ty, float tz);

// Creates a scaling matrix.
mat4_t mat4_scale(float sx, float sy, float sz);

// Creates a rotation matrix from Euler angles (XYZ order).
// Angles are in radians.
mat4_t mat4_rotate_xyz(float rx, float ry, float rz); // rx, ry, rz are angles for X, Y, Z axes

// Creates a rotation matrix around the X-axis. Angle in radians.
mat4_t mat4_rotate_x(float angle_rad);
// Creates a rotation matrix around the Y-axis. Angle in radians.
mat4_t mat4_rotate_y(float angle_rad);
// Creates a rotation matrix around the Z-axis. Angle in radians.
mat4_t mat4_rotate_z(float angle_rad);

// Multiplies two 4x4 matrices (a * b).
mat4_t mat4_multiply(const mat4_t* a, const mat4_t* b);

// Transforms a vec3_t point by a mat4_t (assumes w=1 for point).
// Returns the transformed vec3_t. The w component of the result is ignored for vec3_t.
vec3_t mat4_transform_point(const mat4_t* m, const vec3_t* p);

// Transforms a vec3_t vector by a mat4_t (assumes w=0 for vector/direction).
// Returns the transformed vec3_t. Useful for normals or directions.
vec3_t mat4_transform_vector(const mat4_t* m, const vec3_t* v);


// Creates an asymmetric perspective projection frustum matrix.
// Similar to glFrustum.
// (left, right, bottom, top, nearVal, farVal)
mat4_t mat4_frustum_asymmetric(float l, float r, float b, float t, float n, float f);

// Creates a perspective projection matrix.
// fovy: field of view in Y direction, in radians
// aspect: aspect ratio (width / height)
// n: near clipping plane
// f: far clipping plane
mat4_t mat4_perspective(float fovy_rad, float aspect, float n, float f);


// (Bonus for Task 3, but good to have in math library)
// Structure for a Quaternion
typedef struct {
    float x, y, z, w; // x,y,z are vector part, w is scalar part
} quat_t;

// Quaternion from axis-angle
quat_t quat_from_axis_angle(vec3_t axis, float angle_rad);

// Quaternion to rotation matrix
mat4_t quat_to_mat4(quat_t q);

// Quaternion SLERP
quat_t quat_slerp(quat_t q1, quat_t q2, float t);


#endif // MATH3D_H
