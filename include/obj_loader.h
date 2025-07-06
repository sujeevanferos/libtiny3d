#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include "renderer.h" // For model_t definition

/**
 * @brief Loads a 3D model from OBJ data provided as a string.
 *
 * This is a very simple OBJ parser that supports:
 * - Vertex lines ('v x y z')
 * - Face lines ('f v1 v2 v3 ...') (assuming polygonal faces, generates edges)
 * It does NOT support vertex normals, texture coordinates, material files, etc.
 * It assumes face vertex indices are 1-based.
 *
 * @param obj_data_string A null-terminated string containing the OBJ file content.
 * @return model_t* A pointer to the loaded model_t structure, or NULL on failure.
 *                  The caller is responsible for freeing the model using model_destroy().
 */
model_t* obj_load_from_string(const char* obj_data_string);

#endif // OBJ_LOADER_H
