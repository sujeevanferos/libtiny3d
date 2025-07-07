#include "../include/lighting.h"
#include <math.h> // For fmaxf

// Dot product helper (already in math3d.c but might not be public, or redefine for clarity)
static float _vec3_dot_product(const vec3_t* v1, const vec3_t* v2) {
    return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
}

float calculate_lambertian_intensity(vec3_t surface_normal, vec3_t light_direction) {
    // Ensure vectors are normalized by the caller.
    // The problem statement for Lambert lighting: "intensity = max(0, dot(edge_dir, light_dir))"
    // Here, surface_normal acts as edge_dir.
    // light_direction is typically the vector *from* the surface point *to* the light source.
    // If light_t.direction is defined as direction *of* light rays (e.g. sun direction),
    // then the vector to the light source is -light_t.direction.
    // Let's assume light_direction parameter here is already correctly oriented (vector from surface to light).

    float dot_product = _vec3_dot_product(&surface_normal, &light_direction);
    return fmaxf(0.0f, dot_product);
}

float calculate_total_lighting_intensity(vec3_t surface_normal, const light_t* lights, int num_lights) {
    if (!lights || num_lights <= 0) {
        return 0.5f; // Default ambient intensity if no lights, or 0.0f if strict
    }

    float total_intensity = 0.0f;

    // Ensure surface_normal is normalized (caller should do this, but good practice)
    // vec3_normalize(&surface_normal); // Assuming it's already done by caller

    for (int i = 0; i < num_lights; ++i) {
        if (lights[i].type == LIGHT_TYPE_DIRECTIONAL) {
            // For directional lights, light.direction is usually the direction *of* the light rays.
            // The vector needed for dot product is from surface point *towards* the light source.
            // So, we need to decide convention for light_t.direction.
            // If light_t.direction is "direction light rays are travelling", then vector to light is -lights[i].direction.
            // If light_t.direction is "direction towards the light source from origin", then use it directly if surface is at origin,
            // or calculate appropriately.
            // Let's assume lights[i].direction is already the vector *towards* the light source (normalized).
            // And that surface_normal is the "edge_dir" as per problem spec.

            vec3_t light_dir_normalized = lights[i].direction; // Assume it's already normalized
            // vec3_normalize(&light_dir_normalized); // Ensure normalization if not guaranteed

            // The problem's formula: intensity = max(0, dot(edge_dir, light_dir))
            // Here, surface_normal is edge_dir.
            // And light_dir_normalized is light_dir.
            float intensity = calculate_lambertian_intensity(surface_normal, light_dir_normalized);
            total_intensity += intensity; // Accumulate intensity from each light
        }
        // Add other light types (e.g., point lights) here later
    }

    // Clamp total intensity to [0.0, 1.0]
    if (total_intensity > 1.0f) total_intensity = 1.0f;
    if (total_intensity < 0.0f) total_intensity = 0.0f; // Should not happen with fmaxf(0,...)

    return total_intensity;
}
