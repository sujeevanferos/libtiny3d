#ifndef LIGHTING_H
#define LIGHTING_H

#include "math3d.h" // For vec3_t

// Type of light
typedef enum {
    LIGHT_TYPE_DIRECTIONAL,
    // LIGHT_TYPE_POINT // Future extension
} light_type_t;

// Structure for a light source
typedef struct {
    light_type_t type;
    vec3_t direction; // For directional lights (normalized vector)
    // vec3_t position; // For point lights
    // float intensity; // Base intensity of the light source (e.g., 0.0 to 1.0)
    // vec3_t color; // Color of the light (future extension)
} light_t;

/**
 * @brief Calculates the Lambertian lighting intensity for a given surface normal and light.
 *
 * Intensity = max(0, dot(surface_normal, light_direction))
 * This function assumes a single directional light.
 *
 * @param surface_normal The normal vector of the surface (or edge proxy). Must be normalized.
 * @param light_direction The direction vector of the light. Must be normalized.
 *                        Note: For directional lights, this is the direction *towards* the light source.
 *                        If light.direction is direction *from* light source, use -light.direction.
 * @return float The calculated light intensity (0.0 to 1.0).
 */
float calculate_lambertian_intensity(vec3_t surface_normal, vec3_t light_direction);


/**
 * @brief Calculates the total lighting intensity for a surface normal from multiple light sources.
 *
 * @param surface_normal The normal vector of the surface (or edge proxy). Must be normalized.
 * @param lights Array of light_t sources.
 * @param num_lights Number of lights in the array.
 * @return float The total calculated light intensity (clamped between 0.0 and 1.0).
 */
float calculate_total_lighting_intensity(vec3_t surface_normal, const light_t* lights, int num_lights);


// For wireframe rendering, we don't have surface normals per se for edges.
// We can approximate this:
// 1. Use face normals if faces are defined.
// 2. For an edge, its "direction" could be used, but Lambert lighting is about how much a surface *faces* a light.
//    - One approach: average normals of faces sharing the edge.
//    - Simpler for wireframe: the problem says "If a line is pointing toward the light, it should be bright. Use: intensity = max(0, dot(edge_dir, light_dir))"
//      This means `edge_dir` is the "surface normal" proxy here. This is unusual for Lambert but specified.
//      `edge_dir` should be normalized. `light_dir` should also be normalized.

#endif // LIGHTING_H
