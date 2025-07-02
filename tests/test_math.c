#include <stdio.h>
#include "../include/math3d.h"

int main(){
    vec3_t a = vec3_from_cartesian(1, 0, 0);
    vec3_t b = vec3_from_cartesian(0, 1, 0);

    printf("A: (%.2f, %.2f, %.2f)\n", a.x, a.y, a.z);
    printf("B: (%.2f, %.2f, %.2f)\n", b.x, b.y, b.z);

    for (float t = 0; t <= 1.0f; t += 0.2f){
        vec3_t c = vec3_slerp(a, b, t);
        printf("t=%.1f -> (%.2f, %.2f, %.2f)\n", t, c.x, c.y, c.z);
    }

    return 0;

}