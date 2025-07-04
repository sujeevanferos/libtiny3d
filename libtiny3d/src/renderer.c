#include "renderer.h"
#include <stdlib.h> // For malloc, free, qsort
#include <stdio.h>  // For printf (debugging)
#include <math.h>   // For sqrtf, fabsf

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define FLT_EPSILON 1.19209290E-07F


// --- Model Helper Functions ---
model_t* model_create(int num_vertices, int num_edges) {
    if (num_vertices <= 0 || num_edges < 0) return NULL;

    model_t* model = (model_t*)malloc(sizeof(model_t));
    if (!model) return NULL;

    model->vertices = (vec3_t*)malloc(num_vertices * sizeof(vec3_t));
    if (!model->vertices) {
        free(model);
        return NULL;
    }
    model->num_vertices = num_vertices;

    if (num_edges > 0) {
        model->edges = (int*)malloc(num_edges * 2 * sizeof(int)); // Each edge has 2 vertex indices
        if (!model->edges) {
            free(model->vertices);
            free(model);
            return NULL;
        }
    } else {
        model->edges = NULL;
    }
    model->num_edges = num_edges;

    return model;
}

void model_destroy(model_t* model) {
    if (!model) return;
    free(model->vertices);
    free(model->edges);
    free(model);
}


// --- Core Rendering Functions ---

projected_vertex_t project_vertex(vec3_t local_vertex,
                                  const mat4_t* model_matrix,
                                  const mat4_t* view_matrix,
                                  const mat4_t* projection_matrix,
                                  int screen_width, int screen_height) {
    projected_vertex_t pv;
    pv.is_clipped = 0; // Default to not clipped

    // 1. Local -> World
    vec3_t world_pos = mat4_transform_point(model_matrix, &local_vertex);

    // 2. World -> Camera (View)
    vec3_t camera_pos = mat4_transform_point(view_matrix, &world_pos);

    // 3. Camera -> Clip Space
    float x = camera_pos.x, y = camera_pos.y, z_cam = camera_pos.z;

    vec3_t clip_pos_h;
    clip_pos_h.x = projection_matrix->m[0]*x + projection_matrix->m[4]*y + projection_matrix->m[8]*z_cam  + projection_matrix->m[12];
    clip_pos_h.y = projection_matrix->m[1]*x + projection_matrix->m[5]*y + projection_matrix->m[9]*z_cam  + projection_matrix->m[13];
    clip_pos_h.z = projection_matrix->m[2]*x + projection_matrix->m[6]*y + projection_matrix->m[10]*z_cam + projection_matrix->m[14];
    float w_clip = projection_matrix->m[3]*x + projection_matrix->m[7]*y + projection_matrix->m[11]*z_cam + projection_matrix->m[15];

    pv.position_screen.z = z_cam; // Store camera space Z for depth sorting

    if (w_clip < FLT_EPSILON) {
        pv.is_clipped = 1;
        pv.position_screen.x = -10000; pv.position_screen.y = -10000;
        return pv;
    }

    vec3_t ndc_pos;
    ndc_pos.x = clip_pos_h.x / w_clip;
    ndc_pos.y = clip_pos_h.y / w_clip;
    ndc_pos.z = clip_pos_h.z / w_clip;

    if (ndc_pos.x < -1.0f || ndc_pos.x > 1.0f ||
        ndc_pos.y < -1.0f || ndc_pos.y > 1.0f ||
        ndc_pos.z < -1.0f || ndc_pos.z > 1.0f) {
        pv.is_clipped = 2;
    }

    pv.position_screen.x = (ndc_pos.x + 1.0f) * 0.5f * (float)screen_width;
    pv.position_screen.y = (1.0f - ndc_pos.y) * 0.5f * (float)screen_height;

    vec3_set_cartesian(&(pv.position_screen), pv.position_screen.x, pv.position_screen.y, pv.position_screen.z);

    return pv;
}

// Note: clip_pixel_to_circular_viewport was moved to canvas.c as a static helper
// and is now integrated into set_pixel_f via canvas->active_viewport_radius.


// Structure for an edge to be rendered, including its projected vertices and average depth
typedef struct {
    projected_vertex_t v0;
    projected_vertex_t v1;
    float avg_z; // Average camera-space Z of the edge's endpoints for sorting
    int original_edge_index; // For debugging or advanced usage
} renderable_edge_t;

// Comparison function for qsort to sort edges by average Z (back-to-front)
static int compare_renderable_edges(const void* a, const void* b) {
    renderable_edge_t* edge_a = (renderable_edge_t*)a;
    renderable_edge_t* edge_b = (renderable_edge_t*)b;
    if (edge_a->avg_z < edge_b->avg_z) return 1;
    if (edge_a->avg_z > edge_b->avg_z) return -1;
    return 0;
}


#include "lighting.h" // For lighting calculations

void render_wireframe(canvas_t* canvas,
                      const model_t* model,
                      const mat4_t* model_matrix,
                      const mat4_t* view_matrix,
                      const mat4_t* projection_matrix,
                      const light_t* lights, int num_lights, // Lighting parameters
                      float viewport_radius_param,
                      float line_thickness) {
    if (!canvas || !model || !model->vertices || !model->edges ||
        !model_matrix || !view_matrix || !projection_matrix) {
        // Note: lights can be NULL or num_lights can be 0 for unlit rendering
        fprintf(stderr, "Error: Invalid core arguments to render_wireframe.\n");
        return;
    }
    if (model->num_vertices == 0 || model->num_edges == 0) {
        return;
    }

    canvas_set_circular_viewport(canvas, viewport_radius_param);

    projected_vertex_t* projected_vertices = (projected_vertex_t*)malloc(model->num_vertices * sizeof(projected_vertex_t));
    if (!projected_vertices) {
        fprintf(stderr, "Error: Failed to allocate memory for projected vertices.\n");
        return;
    }
    for (int i = 0; i < model->num_vertices; ++i) {
        projected_vertices[i] = project_vertex(model->vertices[i], model_matrix, view_matrix, projection_matrix, canvas->width, canvas->height);
    }

    renderable_edge_t* edges_to_render = (renderable_edge_t*)malloc(model->num_edges * sizeof(renderable_edge_t));
    if (!edges_to_render) {
        fprintf(stderr, "Error: Failed to allocate memory for renderable edges.\n");
        free(projected_vertices);
        return;
    }

    int current_renderable_edge = 0;
    for (int i = 0; i < model->num_edges; ++i) {
        int idx0 = model->edges[i * 2 + 0];
        int idx1 = model->edges[i * 2 + 1];

        if (idx0 < 0 || idx0 >= model->num_vertices || idx1 < 0 || idx1 >= model->num_vertices) {
            fprintf(stderr, "Warning: Invalid vertex index for edge %d. Skipping.\n", i);
            continue;
        }

        projected_vertex_t pv0 = projected_vertices[idx0];
        projected_vertex_t pv1 = projected_vertices[idx1];

        if (pv0.is_clipped == 1 || pv1.is_clipped == 1) {
            continue;
        }

        edges_to_render[current_renderable_edge].v0 = pv0;
        edges_to_render[current_renderable_edge].v1 = pv1;
        edges_to_render[current_renderable_edge].avg_z = (pv0.position_screen.z + pv1.position_screen.z) * 0.5f;
        edges_to_render[current_renderable_edge].original_edge_index = i;
        current_renderable_edge++;
    }
    int num_actual_renderable_edges = current_renderable_edge;

    qsort(edges_to_render, num_actual_renderable_edges, sizeof(renderable_edge_t), compare_renderable_edges);

    for (int i = 0; i < num_actual_renderable_edges; ++i) {
        renderable_edge_t* edge = &edges_to_render[i];

        if (edge->v0.is_clipped == 0 && edge->v1.is_clipped == 0) {
            float line_intensity = 1.0f; // Default full intensity if no lights

            if (lights && num_lights > 0) {
                // Calculate edge direction in world space for lighting
                // Need original vertices in world space
                vec3_t v0_world = mat4_transform_point(model_matrix, &model->vertices[model->edges[edge->original_edge_index * 2 + 0]]);
                vec3_t v1_world = mat4_transform_point(model_matrix, &model->vertices[model->edges[edge->original_edge_index * 2 + 1]]);

                vec3_t edge_dir_world;
                edge_dir_world.x = v1_world.x - v0_world.x;
                edge_dir_world.y = v1_world.y - v0_world.y;
                edge_dir_world.z = v1_world.z - v0_world.z;
                vec3_normalize(&edge_dir_world); // Normalize the edge direction

                // The problem states: "intensity = max(0, dot(edge_dir, light_dir))"
                // So, edge_dir_world is used as the "surface normal" proxy.
                line_intensity = calculate_total_lighting_intensity(edge_dir_world, lights, num_lights);
            }

            draw_line_f(canvas,
                        edge->v0.position_screen.x, edge->v0.position_screen.y,
                        edge->v1.position_screen.x, edge->v1.position_screen.y,
                        line_thickness,
                        line_intensity);
        }
    }

    free(projected_vertices);
    free(edges_to_render);
}


// Generates a model. Currently returns a CUBE as a placeholder for the soccer ball.
// A proper soccer ball (truncated icosahedron) has 60 vertices and 90 edges.
model_t* generate_soccer_ball() {
    printf("Info: generate_soccer_ball() is returning a CUBE model instead of a soccer ball.\n");

    model_t* cube_model = model_create(8, 12); // 8 vertices, 12 edges for a cube
    if (!cube_model) {
        fprintf(stderr, "Error: Failed to create model for cube in generate_soccer_ball.\n");
        return NULL;
    }

    // Define cube vertices (e.g., side length 2, centered at origin)
    float s = 1.0f; // Half side length, so cube is from -1 to +1
    cube_model->vertices[0] = vec3_create_cartesian(-s, -s, -s);
    cube_model->vertices[1] = vec3_create_cartesian( s, -s, -s);
    cube_model->vertices[2] = vec3_create_cartesian( s,  s, -s);
    cube_model->vertices[3] = vec3_create_cartesian(-s,  s, -s);
    cube_model->vertices[4] = vec3_create_cartesian(-s, -s,  s);
    cube_model->vertices[5] = vec3_create_cartesian( s, -s,  s);
    cube_model->vertices[6] = vec3_create_cartesian( s,  s,  s);
    cube_model->vertices[7] = vec3_create_cartesian(-s,  s,  s);

    // Define cube edges (12 edges)
    int edges_data[] = {
        0,1, 1,2, 2,3, 3,0, // Bottom face
        4,5, 5,6, 6,7, 7,4, // Top face
        0,4, 1,5, 2,6, 3,7  // Connecting sides
    };
    for(int i=0; i < cube_model->num_edges * 2; ++i) { // num_edges is 12, so loop 24 times for pairs
        cube_model->edges[i] = edges_data[i];
    }

    return cube_model;
}
