#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/soccer_loader.h"

mesh_t load_obj(const char* filename) {
    mesh_t mesh = {0};
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "[Error] Cannot open OBJ file: %s\n", filename);
        exit(1);
    }

    // Allocate space for up to 1000 vertices and 1000 faces (adjust as needed)
    mesh.vertices = malloc(sizeof(vec3_t) * 1000);
    mesh.edges = malloc(sizeof(int[2]) * 3000); // Overestimate edges
    int vertex_count = 0;
    int edge_count = 0;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v' && isspace(line[1])) {
            float x, y, z;
            sscanf(line, "v %f %f %f", &x, &y, &z);
            mesh.vertices[vertex_count++] = vec3_from_cartesian(x, y, z);
        } else if (line[0] == 'f') {
            int indices[6];
            int count = sscanf(line, "f %d %d %d %d %d %d",
                               &indices[0], &indices[1], &indices[2],
                               &indices[3], &indices[4], &indices[5]);
            for (int i = 0; i < count; i++) {
                indices[i]--; // Convert to 0-based indexing
            }
            for (int i = 0; i < count; i++) {
                int from = indices[i];
                int to = indices[(i + 1) % count];
                mesh.edges[edge_count][0] = from;
                mesh.edges[edge_count][1] = to;
                edge_count++;
            }
        }
    }

    fclose(file);

    mesh.vertex_count = vertex_count;
    mesh.edge_count = edge_count;

    return mesh;
}
