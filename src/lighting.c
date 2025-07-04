// lighting.c
#include <math.h>
#include "../include/lighting.h"

float compute_lambert_intensity(vec3_t edge_dir, vec3_t light_dir) {
    vec3_t edge_n = vec3_normalize_fast(edge_dir);
    vec3_t light_n = vec3_normalize_fast(light_dir);
    float dot = edge_n.x * light_n.x + edge_n.y * light_n.y + edge_n.z * light_n.z;
    return fmaxf(0.0f, dot);
}
