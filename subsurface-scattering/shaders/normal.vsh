//
//  normal.vsh
//  sub-surface scattering
//
//  Created by Jedd Haberstro on 6/2/11.
//  Copyright 2011 Student. All rights reserved.
//

uniform mat4 modelview;
uniform mat4 projection;
uniform mat3 normal_matrix;

attribute vec4 position;
attribute vec3 normal;

varying vec3 frag_eye_vec;
varying vec3 frag_normal;

void main()
{
    frag_normal = normal_matrix * normal; //(modelview * vec4(normal, 0.0)).xyz;
    frag_eye_vec = vec3(modelview * position);
    gl_Position = projection * modelview * position;
}