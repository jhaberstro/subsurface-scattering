//
//  normal.vsh
//  sub-surface scattering
//
//  Created by Jedd Haberstro on 6/2/11.
//  Copyright 2011 Student. All rights reserved.
//

uniform mat4 modelview;
uniform mat4 projection;
uniform vec3 light_pos;
uniform vec3 diffuse;
uniform vec3 ambient;
uniform vec3 specular;
uniform float shininess;

varying vec3 frag_eye_vec;
varying vec3 frag_normal;

void main()
{
    vec3 L = normalize(light_pos - frag_eye_vec);  
    vec3 N = normalize(frag_normal);
    vec3 R = reflect(-L, N);
    vec3 E = normalize(-frag_eye_vec);
    
    float ld = max(dot(N, L), 0.0);
    float ls = pow(max(dot(R, E), 0.0), 0.3 * shininess);
    
    gl_FragColor = vec4(ambient + (ld * diffuse) + (ls * specular), 1.0);
}