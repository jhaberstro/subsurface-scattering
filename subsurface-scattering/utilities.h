//
//  path_utilities.h
//  sub-surface scattering
//
//  Created by Jedd Haberstro on 5/9/11.
//  Copyright 2011 Student. All rights reserved.
//

#ifndef UTILITIES_H
#define UTILITIES_H

#import <OpenGL/gl.h>

struct attribute
{
    GLuint index;
    char const* name;
};

char const* path_for_resource_and_type(char const* resource, char const* type);

char const* path_for_resource_and_type(char const* file);

void load_png(char const* filename);

void save_framebuffer(char const* filename);

GLuint load_shader(char const* vsh, char const* fsh, attribute const* attributes, int n);

#endif // PATH_UTILITIES_H