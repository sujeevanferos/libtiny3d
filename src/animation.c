#include "../include/animation.h"
#include <math.h> // For powf if used, though direct expansion is better

vec3_t bezier_cubic(vec3_t p0, vec3_t p1, vec3_t p2, vec3_t p3, float t) {
    vec3_t result;

    // Clamp t to [0, 1] for standard Bézier segment
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    // P(t) = (1-t)^3*P0 + 3*(1-t)^2*t*P1 + 3*(1-t)*t^2*P2 + t^3*P3
    // P(t) = uuu*P0 + 3*uu*t*P1 + 3*u*tt*P2 + ttt*P3

    // Scale P0
    float p0x = uuu * p0.x;
    float p0y = uuu * p0.y;
    float p0z = uuu * p0.z;

    // Scale P1
    float p1x = 3.0f * uu * t * p1.x;
    float p1y = 3.0f * uu * t * p1.y;
    float p1z = 3.0f * uu * t * p1.z;

    // Scale P2
    float p2x = 3.0f * u * tt * p2.x;
    float p2y = 3.0f * u * tt * p2.y;
    float p2z = 3.0f * u * tt * p2.z;

    // Scale P3
    float p3x = ttt * p3.x;
    float p3y = ttt * p3.y;
    float p3z = ttt * p3.z;

    // Sum scaled points
    float final_x = p0x + p1x + p2x + p3x;
    float final_y = p0y + p1y + p2y + p3y;
    float final_z = p0z + p1z + p2z + p3z;

    vec3_set_cartesian(&result, final_x, final_y, final_z);

    return result;
}

// mat4_t get_animated_model_matrix(const animatable_object_t* object, float current_time) {
//     if (!object) return mat4_identity();

//     mat4_t final_transform = object->base_transform;

//     if (object->translation_path) {
//         float t = fmodf(current_time, object->translation_path->duration) / object->translation_path->duration;
//         if (object->translation_path->duration <= 0.0f) t = 0.0f; // Avoid division by zero

//         vec3_t translation = bezier_cubic(
//             object->translation_path->control_points[0],
//             object->translation_path->control_points[1],
//             object->translation_path->control_points[2],
//             object->translation_path->control_points[3],
//             t
//         );
//         mat4_t translation_matrix = mat4_translate(translation.x, translation.y, translation.z);
//         final_transform = mat4_multiply(&translation_matrix, &final_transform); // Apply translation first relative to base
//     }

//     // TODO: Add rotation and scale animations if defined
//     // For rotation, this might involve slerping quaternions derived from bezier curves,
//     // or interpreting bezier curve output as euler angles (prone to gimbal lock).

//     return final_transform;
// }
