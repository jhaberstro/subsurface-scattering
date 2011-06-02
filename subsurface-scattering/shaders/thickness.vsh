//
//  thickness.vsh
//  sub-surface scattering
//
//  Created by Jedd Haberstro on 6/2/11.
//  Copyright 2011 Student. All rights reserved.
//

uniform mat4 modelview;
uniform mat4 projection;
attribute vec4 position;
attribute vec3 normal;

void main() {
    gl_Position = projection * modelview * position;
}