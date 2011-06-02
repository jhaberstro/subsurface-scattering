//
//  sss.fsh
//  sub-surface scattering
//
//  Created by Jedd Haberstro on 6/2/11.
//  Copyright 2011 Student. All rights reserved.
//

#define CONSTANT_ATTENUATION 1.0
#define LINEAR_ATTENUATION 0.7
#define QUADRATIC_ATTENUATION 1.8

uniform vec4 light_pos; // world space


uniform sampler2D thickness_texture;
uniform vec4 mat_diffuse;
uniform vec4 light_diffuse;
uniform vec3 ambient_color;
uniform vec3 specular;
uniform float shininess;
uniform vec2 screen_size;
uniform float ambient;
uniform float power;
uniform float distortion;
uniform float scale;
uniform float light_radius;

varying vec3 frag_eye_pos;
varying vec3 frag_eye_dir;
varying vec3 frag_eye_normal;
varying vec3 frag_eye_light_normal;
varying vec3 frag_eye_light_pos;

float saturate(float val) {
    return clamp(val, 0.0, 1.0);
}

vec3 phong_shade(vec3 light_pos, vec3 eye, vec3 normal, float attentuation) {
    vec3 L = normalize(light_pos - eye);  
    vec3 N = normalize(normal);
    vec3 R = reflect(-L, N);
    vec3 E = normalize(-eye);
    
    float ld = max(dot(N, L), 0.0);
    float ls = pow(max(dot(R, E), 0.0), 0.3 * shininess);
    return (ambient_color + (ld * mat_diffuse.xyz) + (ls * specular)) * attentuation;
}

vec3 blinn_shade(vec3 light_pos, vec3 eye, vec3 normal, float attentuation) {
    vec3 L = normalize(light_pos - eye);  
    vec3 N = normalize(normal);
    vec3 V = -normalize(eye);
    float dot = max(dot(N, L), 0.0);
    vec3 c = mat_diffuse.xyz * light_diffuse.xyz * dot;
    vec3 H = normalize(V + L);
    vec3 specular = vec3(clamp(6.0 * pow(max(dot(N, H), 0.0), shininess), 0.0, 1.0));
    return (c + specular) * attentuation;
}

vec4 sss(float thickness, float attentuation) {
    vec3 light = normalize(frag_eye_light_normal + (frag_eye_normal * distortion));
    float dot = pow(saturate(dot(frag_eye_dir, -light)), power) * scale;
    float lt = attentuation * (dot + ambient) * thickness;
    //lt = 1.0 - lt;
    return (mat_diffuse * light_diffuse * lt);
}

void main()
{
    vec4 texture_val = texture2D(thickness_texture, gl_FragCoord.xy / screen_size);
    float thickness = abs(texture_val.r);
    if (thickness <= 0.0) {
        discard;
    }
    
    // THIS NEXT LINE IS HACK
    // The SSS algorithm expects the thickness map to have thinn values
    // be whiter (ie, closer to 1) and thick values closer to black (ie, closer to 0).
    // However, the depth value thickness computation I'm using is the opposite,
    // so I need to adjust the values appropriately. I coefficient multiplier I found
    // but experimentation. It has no real meaning (let's call it "thickness intensity value" :P)
    thickness = ((1.0 - thickness) * 0.05);
        
    vec3 light_dir = (frag_eye_light_pos - frag_eye_pos) / light_radius;
    float light_attentuation = max(1.0 - dot(light_dir, light_dir), 0.0); //1.0 / (CONSTANT_ATTENUATION + LINEAR_ATTENUATION * d + QUADRATIC_ATTENUATION * d * d);

    //vec3 phong_color = phong_shade(frag_eye_light_pos, frag_eye_pos, frag_eye_normal, light_attentuation);
    vec3 blinn_color = blinn_shade(frag_eye_light_pos, frag_eye_pos, frag_eye_normal, light_attentuation);
    gl_FragColor = (vec4(blinn_color, 1.0)) + sss(thickness, light_attentuation);
    //gl_FragColor = vec4(phong_color, 1.0) * light_diffuse * lt;
}
