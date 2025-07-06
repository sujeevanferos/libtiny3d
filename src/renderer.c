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


#include "obj_loader.h" // For obj_load_from_string

// Generates a soccer ball model by loading from embedded OBJ data.
model_t* generate_soccer_ball() {
    printf("Info: generate_soccer_ball() loading model from OBJ data.\n");

    // OBJ data for a soccer ball (truncated icosahedron)
    // Provided by user. Note: OBJ indices are 1-based.
    const char* soccer_ball_obj_data =
        "v 0.0 0.0 1.021\n"
        "v 0.4035482 0.0 0.9378643\n"
        "v -0.2274644 0.3333333 0.9378643\n"
        "v -0.1471226 -0.375774 0.9378643\n"
        "v 0.579632 0.3333333 0.7715933\n"
        "v 0.5058321 -0.375774 0.8033483\n"
        "v -0.6020514 0.2908927 0.7715933\n"
        "v -0.05138057 0.6666667 0.7715933\n"
        "v 0.1654988 -0.6080151 0.8033483\n"
        "v -0.5217096 -0.4182147 0.7715933\n"
        "v 0.8579998 0.2908927 0.4708062\n"
        "v 0.3521676 0.6666667 0.6884578\n"
        "v 0.7841999 -0.4182147 0.5025612\n"
        "v -0.657475 0.5979962 0.5025612\n"
        "v -0.749174 -0.08488134 0.6884578\n"
        "v -0.3171418 0.8302373 0.5025612\n"
        "v 0.1035333 -0.8826969 0.5025612\n"
        "v -0.5836751 -0.6928964 0.4708062\n"
        "v 0.8025761 0.5979962 0.2017741\n"
        "v 0.9602837 -0.08488134 0.3362902\n"
        "v 0.4899547 0.8302373 0.3362902\n"
        "v 0.7222343 -0.6928964 0.2017741\n"
        "v -0.8600213 0.5293258 0.1503935\n"
        "v -0.9517203 -0.1535518 0.3362902\n"
        "v -0.1793548 0.993808 0.1503935\n"
        "v 0.381901 -0.9251375 0.2017741\n"
        "v -0.2710537 -0.9251375 0.3362902\n"
        "v -0.8494363 -0.5293258 0.2017741\n"
        "v 0.8494363 0.5293258 -0.2017741\n"
        "v 1.007144 -0.1535518 -0.06725804\n"
        "v 0.2241935 0.993808 0.06725804\n"
        "v 0.8600213 -0.5293258 -0.1503935\n"
        "v -0.7222343 0.6928964 -0.2017741\n"
        "v -1.007144 0.1535518 0.06725804\n"
        "v -0.381901 0.9251375 -0.2017741\n"
        "v 0.1793548 -0.993808 -0.1503935\n"
        "v -0.2241935 -0.993808 -0.06725804\n"
        "v -0.8025761 -0.5979962 -0.2017741\n"
        "v 0.5836751 0.6928964 -0.4708062\n"
        "v 0.9517203 0.1535518 -0.3362902\n"
        "v 0.2710537 0.9251375 -0.3362902\n"
        "v 0.657475 -0.5979962 -0.5025612\n"
        "v -0.7841999 0.4182147 -0.5025612\n"
        "v -0.9602837 0.08488134 -0.3362902\n"
        "v -0.1035333 0.8826969 -0.5025612\n"
        "v 0.3171418 -0.8302373 -0.5025612\n"
        "v -0.4899547 -0.8302373 -0.3362902\n"
        "v -0.8579998 -0.2908927 -0.4708062\n"
        "v 0.5217096 0.4182147 -0.7715933\n"
        "v 0.749174 0.08488134 -0.6884578\n"
        "v 0.6020514 -0.2908927 -0.7715933\n"
        "v -0.5058321 0.375774 -0.8033483\n"
        "v -0.1654988 0.6080151 -0.8033483\n"
        "v 0.05138057 -0.6666667 -0.7715933\n"
        "v -0.3521676 -0.6666667 -0.6884578\n"
        "v -0.579632 -0.3333333 -0.7715933\n"
        "v 0.1471226 0.375774 -0.9378643\n"
        "v 0.2274644 -0.3333333 -0.9378643\n"
        "v -0.4035482 0.0 -0.9378643\n"
        "v 0.0 0.0 -1.021\n"
        "f 1 4 9 6 2\n"
        "f 3 8 16 14 7\n"
        "f 5 11 19 21 12\n"
        "f 10 15 24 28 18\n"
        "f 13 22 32 30 20\n"
        "f 17 27 37 36 26\n"
        "f 23 33 43 44 34\n"
        "f 25 31 41 45 35\n"
        "f 29 40 50 49 39\n"
        "f 38 48 56 55 47\n"
        "f 42 46 54 58 51\n"
        "f 52 53 57 60 59\n"
        "f 1 2 5 12 8 3\n"
        "f 1 3 7 15 10 4\n"
        "f 2 6 13 20 11 5\n"
        "f 4 10 18 27 17 9\n"
        "f 6 9 17 26 22 13\n"
        "f 7 14 23 34 24 15\n"
        "f 8 12 21 31 25 16\n"
        "f 11 20 30 40 29 19\n"
        "f 14 16 25 35 33 23\n"
        "f 18 28 38 47 37 27\n"
        "f 19 29 39 41 31 21\n"
        "f 22 26 36 46 42 32\n"
        "f 24 34 44 48 38 28\n"
        "f 30 32 42 51 50 40\n"
        "f 33 35 45 53 52 43\n"
        "f 36 37 47 55 54 46\n"
        "f 39 49 57 53 45 41\n"
        "f 43 52 59 56 48 44\n"
        "f 49 50 51 58 60 57\n"
        "f 54 55 56 59 60 58\n";

    model_t* model = obj_load_from_string(soccer_ball_obj_data);

    if (!model) {
        fprintf(stderr, "Failed to load soccer ball model from OBJ data. Returning a cube as fallback.\n");
        // Fallback to cube if OBJ loading fails
        model_t* cube_model = model_create(8, 12);
        if (!cube_model) {
             fprintf(stderr, "Error: Failed to create fallback cube model.\n");
            return NULL;
        }
        float s = 1.0f;
        cube_model->vertices[0] = vec3_create_cartesian(-s, -s, -s);
        cube_model->vertices[1] = vec3_create_cartesian( s, -s, -s);
        cube_model->vertices[2] = vec3_create_cartesian( s,  s, -s);
        cube_model->vertices[3] = vec3_create_cartesian(-s,  s, -s);
        cube_model->vertices[4] = vec3_create_cartesian(-s, -s,  s);
        cube_model->vertices[5] = vec3_create_cartesian( s, -s,  s);
        cube_model->vertices[6] = vec3_create_cartesian( s,  s,  s);
        cube_model->vertices[7] = vec3_create_cartesian(-s,  s,  s);
        int edges_data[] = {0,1, 1,2, 2,3, 3,0, 4,5, 5,6, 6,7, 7,4, 0,4, 1,5, 2,6, 3,7};
        for(int i=0; i < cube_model->num_edges * 2; ++i) cube_model->edges[i] = edges_data[i];
        return cube_model;
    }

    printf("Successfully loaded model: %d vertices, %d edges.\n", model->num_vertices, model->num_edges);
    return model;
}
