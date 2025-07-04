#ifndef ANIMATION_H
#define ANIMATION_H

#include "math3d.h" // For vec3_t
#include "renderer.h" // For model_t

/**
 * @brief Calculates a point on a cubic Bézier curve.
 *
 * P(t) = (1-t)^3 * P0 + 3*(1-t)^2*t * P1 + 3*(1-t)*t^2 * P2 + t^3 * P3
 *
 * @param p0 The first control point (start point).
 * @param p1 The second control point.
 * @param p2 The third control point.
 * @param p3 The fourth control point (end point).
 * @param t The interpolation parameter, typically from 0.0 to 1.0.
 * @return vec3_t The interpolated point on the curve.
 */
vec3_t bezier_cubic(vec3_t p0, vec3_t p1, vec3_t p2, vec3_t p3, float t);

// Structure to define an animation path using a Bézier curve
typedef struct {
    vec3_t control_points[4]; // P0, P1, P2, P3
    float duration;           // Duration of one full animation loop in seconds
    // Add other properties like looping behavior, etc. if needed
} bezier_animation_path_t;


// Structure for an animatable object in the scene
typedef struct {
    model_t* model; // The 3D model to animate
    mat4_t base_transform; // Initial static transform (e.g. scaling, initial rotation)

    // Animation properties
    bezier_animation_path_t* translation_path; // Optional path for translation
    // bezier_animation_path_t* rotation_path; // More complex: for animating rotation target/axis
    // bezier_animation_path_t* scale_path; // For animating scale

    // Current animation state (could be managed externally or here)
    // float current_time_in_animation; // For external time management

} animatable_object_t;


// Function to update an object's model matrix based on animation time
// mat4_t get_animated_model_matrix(const animatable_object_t* object, float current_time);

#endif // ANIMATION_H
