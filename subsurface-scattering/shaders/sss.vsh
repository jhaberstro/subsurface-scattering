//
//  sss.vsh
//  sub-surface scattering
//
//  Created by Jedd Haberstro on 6/2/11.
//  Copyright 2011 Student. All rights reserved.
//

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 light_pos; // world space

attribute vec4 position;
attribute vec3 normal;

varying vec3 frag_eye_pos;
varying vec3 frag_eye_dir;
varying vec3 frag_eye_normal;
varying vec3 frag_eye_light_normal;
varying vec3 frag_eye_light_pos;


void main()
{    
    frag_eye_pos = (modelview * position).xyz;
    frag_eye_dir = normalize(-frag_eye_pos);
    frag_eye_normal = normalize((modelview * vec4(normal, 0.0)).xyz); 
    
    frag_eye_light_pos = light_pos.xyz; // <-- assumes view matrix == identity matrix   //(modelview * light_pos).xyz;
    frag_eye_light_normal = normalize(frag_eye_light_pos - frag_eye_pos);
    
    
    gl_Position = projection * modelview * position;
}
