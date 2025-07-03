#include <stdio.h>
#include "../include/math3d.h"



int main(){
    vec3_t cube[8] = {
    {-1, -1, -1, 0,0,0}, {1, -1, -1, 0,0,0}, {1, 1, -1, 0,0,0}, {-1, 1, -1, 0,0,0},
    {-1, -1, 1, 0,0,0}, {1, -1, 1, 0,0,0}, {1, 1, 1, 0,0,0}, {-1, 1, 1, 0,0,0}
    };

    for (int i = 0; i < 8; i++){
        vec3_update_spherical(&cube[i]);
    }

    int edges[12][2] = {
        {0,1}, {1,2}, {2,3}, {3,0},
        {4,5}, {5,6}, {6,7}, {7,4},
        {0,4}, {1,5}, {2,6}, {3,7}
    };
    mat4_t model = mat4_mul(
        mat4_translate(0, 0, -5),
        mat4_rotate_xyz(0.5f, 0.5f, 0)
    );

    for (int i = 0; i < 12; i++){
        vec3_t a = mat4_mul_vec3(model, cube[edges[i][0]]);
        vec3_t b = mat4_mul_vec3(model, cube[edges[i][1]]);

        printf("Edge %2d: (%.2f, %.2f, %.2f) -> (%.2f, %.2f, %.2f)\n",
        i, (double)a.x, (double)a.y, (double)a.z, (double)b.x, (double)b.y, (double)b.z);
    }

    return 0;
}