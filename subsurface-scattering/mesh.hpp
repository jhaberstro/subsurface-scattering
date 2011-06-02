//
//  mesh.hpp
//  sub-surface scattering
//
//  Created by Jedd Haberstro on 5/16/11.
//  Copyright 2011 Student. All rights reserved.
//

#ifndef MESH_HPP
#define MESH_HPP

#include <OpenGL/gl.h>

struct mesh
{
    GLuint vert_vbo;
    GLuint norm_vbo;
    GLuint ibo;
    int num_tris;
    bool normals;
};

mesh* load_mesh_ctm(char const* filepath);

void draw_mesh(mesh const* mesh, GLuint pos_attrib, GLint normal_attrib = -1);

#endif // MESH_HPP