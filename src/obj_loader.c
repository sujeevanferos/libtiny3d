#include "../include/obj_loader.h"
#include "../include/math3d.h" // For vec3_create_cartesian
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 256
#define INITIAL_VERTICES_CAPACITY 64
#define INITIAL_EDGES_CAPACITY 128

// Helper structure for dynamic arrays during parsing
typedef struct {
    vec3_t* data;
    int count;
    int capacity;
} vec3_dynamic_array_t;

typedef struct {
    int* data; // Stores pairs of indices for edges
    int count; // Number of edges (each edge is 2 ints)
    int capacity; // Capacity in terms of number of edges
} edge_dynamic_array_t;

static int vec3_da_init(vec3_dynamic_array_t* arr, int initial_capacity) {
    arr->data = (vec3_t*)malloc(initial_capacity * sizeof(vec3_t));
    if (!arr->data) return 0;
    arr->count = 0;
    arr->capacity = initial_capacity;
    return 1;
}

static int vec3_da_push(vec3_dynamic_array_t* arr, vec3_t val) {
    if (arr->count >= arr->capacity) {
        int new_capacity = arr->capacity * 2;
        vec3_t* new_data = (vec3_t*)realloc(arr->data, new_capacity * sizeof(vec3_t));
        if (!new_data) return 0;
        arr->data = new_data;
        arr->capacity = new_capacity;
    }
    arr->data[arr->count++] = val;
    return 1;
}

static void vec3_da_free(vec3_dynamic_array_t* arr) {
    free(arr->data);
    arr->data = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

static int edge_da_init(edge_dynamic_array_t* arr, int initial_capacity) {
    arr->data = (int*)malloc(initial_capacity * 2 * sizeof(int)); // 2 ints per edge
    if (!arr->data) return 0;
    arr->count = 0;
    arr->capacity = initial_capacity;
    return 1;
}

// Adds an edge (v_idx1, v_idx2)
static int edge_da_push(edge_dynamic_array_t* arr, int v_idx1, int v_idx2) {
    if (arr->count >= arr->capacity) {
        int new_capacity = arr->capacity * 2;
        int* new_data = (int*)realloc(arr->data, new_capacity * 2 * sizeof(int));
        if (!new_data) return 0;
        arr->data = new_data;
        arr->capacity = new_capacity;
    }
    arr->data[arr->count * 2 + 0] = v_idx1;
    arr->data[arr->count * 2 + 1] = v_idx2;
    arr->count++;
    return 1;
}

static void edge_da_free(edge_dynamic_array_t* arr) {
    free(arr->data);
    arr->data = NULL;
    arr->count = 0;
    arr->capacity = 0;
}


model_t* obj_load_from_string(const char* obj_data_string) {
    if (!obj_data_string) return NULL;

    vec3_dynamic_array_t vertices_da;
    edge_dynamic_array_t edges_da;

    if (!vec3_da_init(&vertices_da, INITIAL_VERTICES_CAPACITY)) return NULL;
    if (!edge_da_init(&edges_da, INITIAL_EDGES_CAPACITY)) {
        vec3_da_free(&vertices_da);
        return NULL;
    }

    char line_buffer[MAX_LINE_LENGTH];
    const char* current_pos = obj_data_string;

    while (*current_pos) {
        const char* next_newline = strchr(current_pos, '\n');
        size_t line_len;
        if (next_newline) {
            line_len = next_newline - current_pos;
            if (line_len >= MAX_LINE_LENGTH) line_len = MAX_LINE_LENGTH - 1;
            strncpy(line_buffer, current_pos, line_len);
            line_buffer[line_len] = '\0';
            current_pos = next_newline + 1;
        } else {
            // Last line
            strncpy(line_buffer, current_pos, MAX_LINE_LENGTH -1);
            line_buffer[MAX_LINE_LENGTH-1] = '\0'; // Ensure null termination
            line_len = strlen(line_buffer); // update line_len
            current_pos += line_len; // Move to end of string
        }

        // Trim trailing carriage return if present (Windows-style newlines)
        if (line_len > 0 && line_buffer[line_len-1] == '\r') {
            line_buffer[line_len-1] = '\0';
        }


        if (strncmp(line_buffer, "v ", 2) == 0) { // Vertex
            float x, y, z;
            if (sscanf(line_buffer, "v %f %f %f", &x, &y, &z) == 3) {
                if (!vec3_da_push(&vertices_da, vec3_create_cartesian(x, y, z))) {
                    fprintf(stderr, "OBJ Loader: Failed to push vertex.\n");
                    goto error;
                }
            }
        } else if (strncmp(line_buffer, "f ", 2) == 0) { // Face
            int face_verts_indices[32]; // Max 32 verts per face for this simple parser
            int num_face_verts = 0;
            char* token_start = line_buffer + 2; // Skip "f "

            while (*token_start && num_face_verts < 32) {
                while (*token_start == ' ') token_start++; // Skip spaces
                if (*token_start == '\0') break;

                int v_idx;
                // OBJ faces can be v, v/vt, v/vt/vn, or v//vn. We only care about v.
                if (sscanf(token_start, "%d", &v_idx) == 1) {
                    face_verts_indices[num_face_verts++] = v_idx;
                }
                // Move to next token part
                while (*token_start && *token_start != ' ') token_start++;
            }

            if (num_face_verts >= 3) {
                for (int i = 0; i < num_face_verts; ++i) {
                    // OBJ indices are 1-based, convert to 0-based
                    int idx1 = face_verts_indices[i] - 1;
                    int idx2 = face_verts_indices[(i + 1) % num_face_verts] - 1;

                    // Basic bounds check
                    if (idx1 < 0 || idx1 >= vertices_da.count || idx2 < 0 || idx2 >= vertices_da.count) {
                        fprintf(stderr, "OBJ Loader: Vertex index out of bounds in face definition. Line: %s\n", line_buffer);
                        continue; // Skip this edge
                    }

                    // TODO: Add check for duplicate edges if desired, for now allow them
                    if (!edge_da_push(&edges_da, idx1, idx2)) {
                        fprintf(stderr, "OBJ Loader: Failed to push edge.\n");
                        goto error;
                    }
                }
            }
        }
        // Ignore other lines (comments '#', 'vt', 'vn', 's', 'o', 'g', 'usemtl', 'mtllib')
    }

    // Create model_t from dynamic arrays
    model_t* model = model_create(vertices_da.count, edges_da.count);
    if (!model) goto error;

    memcpy(model->vertices, vertices_da.data, vertices_da.count * sizeof(vec3_t));
    memcpy(model->edges, edges_da.data, edges_da.count * 2 * sizeof(int));

    vec3_da_free(&vertices_da);
    edge_da_free(&edges_da);
    return model;

error:
    vec3_da_free(&vertices_da);
    edge_da_free(&edges_da);
    return NULL;
}
