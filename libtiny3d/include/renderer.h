#ifndef RENDERER_H
#define RENDERER_H

#include "math3d.h"
#include "canvas.h"
#include "lighting.h" // Added for light_t parameter in render_wireframe

// Structure to hold a 3D model/object for wireframe rendering
// Consists of vertices and edges (indices into the vertex array)
typedef struct {
    vec3_t* vertices;       // Array of vertices (local coordinates)
    int num_vertices;

    int* edges;             // Array of edge pairs (e.g., [v1_idx, v2_idx, v1_idx, v2_idx, ...])
    int num_edges;          // Number of edges (each edge is 2 indices)

    // Optional: per-vertex normals, colors, texture coordinates for future expansion
} model_t;

// Structure to hold vertex information after projection and ready for screen space
typedef struct {
    vec3_t position_screen; // x, y screen coordinates; z for depth (after perspective divide)
    int is_clipped;         // Flag indicating if the vertex is outside the viewport or near/far planes
} projected_vertex_t;


// --- Function Prototypes ---

/**
 * @brief Projects a single 3D vertex through the full transformation pipeline.
 *
 * Transforms a vertex from model space to screen space.
 * Pipeline: Local -> World (model_matrix) -> Camera (view_matrix) -> Projection (projection_matrix) -> Screen
 * The result includes perspective division.
 *
 * @param vertex The 3D vertex in local model coordinates.
 * @param model_matrix The model-to-world transformation matrix.
 * @param view_matrix The world-to-camera (view) transformation matrix.
 * @param projection_matrix The camera-to-clip space (projection) transformation matrix.
 * @param screen_width The width of the target screen/canvas.
 * @param screen_height The height of the target screen/canvas.
 * @return projected_vertex_t The vertex projected to screen space, with its depth value (z).
 *                            The is_clipped flag will be set if it's outside conservative frustum checks.
 */
projected_vertex_t project_vertex(vec3_t local_vertex,
                                  const mat4_t* model_matrix,
                                  const mat4_t* view_matrix,
                                  const mat4_t* projection_matrix,
                                  int screen_width, int screen_height);

// Note: clip_pixel_to_circular_viewport was moved to canvas.c as a static helper
// and is now integrated into set_pixel_f.

/**
 * @brief Renders a 3D model as a wireframe on the canvas.
 *
 * - Transforms all vertices of the model using model, view, and projection matrices.
 * - Performs back-to-front sorting of lines/edges based on their average depth.
 * - Draws each edge using draw_line_f from canvas.c.
 * - Lines should be clipped (or individual pixels when drawing lines) to a circular viewport.
 *
 * @param canvas The canvas to draw on.
 * @param model A pointer to the model_t data (vertices and edges).
 * @param model_matrix Model transformation matrix.
 * @param view_matrix View (camera) transformation matrix.
 * @param projection_matrix Projection transformation matrix.
 * @param viewport_radius Radius of the circular viewport. If 0, defaults to half of min(canvas_width, canvas_height).
 * @param line_thickness Thickness for drawing lines.
 */
void render_wireframe(canvas_t* canvas,
                      const model_t* model,
                      const mat4_t* model_matrix,
                      const mat4_t* view_matrix,
                      const mat4_t* projection_matrix,
                       const light_t* lights, int num_lights, // Lighting parameters
                      float viewport_radius,
                      float line_thickness);


// Helper functions for model_t (e.g., creation, destruction)
model_t* model_create(int num_vertices, int num_edges);
void model_destroy(model_t* model);

// (Task 3.3.1) Generates a soccer ball (truncated icosahedron) model.
// This function will populate a model_t structure.
// Returns a pointer to a new model_t, or NULL on failure. Caller must free.
model_t* generate_soccer_ball();


#endif // RENDERER_H
