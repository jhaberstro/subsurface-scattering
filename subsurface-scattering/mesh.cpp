//
//  mesh.cpp
//  sub-surface scattering
//
//  Created by Jedd Haberstro o    5/16/11.
//  Copyright 2011 Student. All rights reserved.
//

#include "mesh.hpp"
#include "utilities.h"
#include "openctm.h"
#include <cassert>

#include "glm/glm.hpp"
#include "glm/ext.hpp"

struct vec3
{
    float x, y, z;
};

mesh* load_mesh_ctm(char const* filepath) {
    CTMcontext context = ctmNewContext(CTM_IMPORT);
    ctmLoad(context, path_for_resource_and_type(filepath));
    assert(ctmGetError(context) == CTM_NONE);
    
    CTMuint tri_count = ctmGetInteger(context, CTM_TRIANGLE_COUNT);;
    CTMuint num_vertices = ctmGetInteger(context, CTM_VERTEX_COUNT);
    CTMuint num_indices = tri_count * 3;
    CTMuint num_normals = 0;
    CTMfloat const* vertices = ctmGetFloatArray(context, CTM_VERTICES);
    CTMuint const* indices = ctmGetIntegerArray(context, CTM_INDICES);
    CTMfloat const* normals = 0;
    if (ctmGetInteger(context, CTM_HAS_NORMALS) == CTM_TRUE) {
        num_normals = num_vertices;
        normals = ctmGetFloatArray(context, CTM_NORMALS);
    }
    
    mesh* new_mesh = new mesh;
    new_mesh->num_tris = tri_count;
    new_mesh->normals = num_normals > 0;

    // Create buffers
    glGenBuffers(1, &(new_mesh->vert_vbo));
    glBindBuffer(GL_ARRAY_BUFFER, new_mesh->vert_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * num_vertices, vertices, GL_STATIC_DRAW);
    
    glGenBuffers(1, &(new_mesh->norm_vbo));
    glBindBuffer(GL_ARRAY_BUFFER, new_mesh->norm_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * num_vertices, normals, GL_STATIC_DRAW);
    
    assert(indices);
    glGenBuffers(1, &(new_mesh->ibo));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, new_mesh->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CTMuint) * num_indices, indices, GL_STATIC_DRAW);
    
    ctmFreeContext(context);
    
    return new_mesh;
}

void draw_mesh(mesh const* mesh, GLuint pos_attrib, GLint normal_attrib) {
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vert_vbo);
    glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0);
    glEnableVertexAttribArray(pos_attrib);
    
    if (mesh->normals && normal_attrib >= 0) {
        glBindBuffer(GL_ARRAY_BUFFER, mesh->norm_vbo);
        glVertexAttribPointer(normal_attrib, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0);
        glEnableVertexAttribArray(normal_attrib);
    }
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
    glDrawElements(GL_TRIANGLES, mesh->num_tris * 3, GL_UNSIGNED_INT, 0);
    
    glDisableVertexAttribArray(pos_attrib);
    glDisableVertexAttribArray(normal_attrib);
}