//
//  main.cpp
//  sub-surface scattering
//
//  Created by Jedd Haberstro on 5/17/11.
//  Copyright 2011 Student. All rights reserved.
//

#include <SFML/Window.hpp>
#include "mesh.hpp"
#include "utilities.h"
#include "openctm.h"

#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include "pez/pez.h"

enum
{
    WINDOW_WIDTH = PEZ_VIEWPORT_WIDTH,
    WINDOW_HEIGHT = PEZ_VIEWPORT_HEIGHT
};

enum
{
    ATTRIB_VERTEX,
    ATTRIB_TEXCOORD,
    ATTRIB_NORMAL,
    NUM_ATTRIBUTES
};


GLuint thickness_program;
GLuint sss_program;
GLuint normal_program;
GLuint scene_fbo;
GLuint scene_texture;
GLuint quad_vbo;
float obj_rotation = 45.0f;
float light_rotation = 0.0f;
mesh* dragon_mesh;
mesh* buddha_mesh;
mesh* sphere_mesh;
glm::vec3 obj_position;
glm::vec3 light_position;
glm::vec3 light_translation;


void create_fbo(GLuint& fbo, GLuint& depth_texture, GLenum internalFormat, GLenum format) {
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        
    glGenTextures(1, &depth_texture);
    glBindTexture(GL_TEXTURE_2D, depth_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, WINDOW_WIDTH, WINDOW_HEIGHT, 0, format, GL_FLOAT, 0);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    assert(GL_NO_ERROR == glGetError());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depth_texture, 0);
    
    //GLuint depthRenderbuffer;
    //glGenRenderbuffers(1, &depthRenderbuffer);
    //glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
    //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, WINDOW_WIDTH, WINDOW_HEIGHT);
    //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE) {
        printf("failed to make complete framebuffer object %x", status);
        //assert(false);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

GLuint create_quad(float left, float bottom, float right, float top)
{
    float quad[] = {
        left, bottom, 0, 0,
        left, top, 0, 1,
        right, top, 1, 1,
        right, top, 1, 1,
        right, bottom, 1, 0,
        left, bottom, 0, 0
    };
    
    GLuint handle;
    glGenBuffers(1, &handle);
    glBindBuffer(GL_ARRAY_BUFFER, handle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), &quad[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    return handle;
}

void draw_quad(GLuint vbo, GLuint pos_attrib, GLuint tex_attrib) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(pos_attrib);
    glEnableVertexAttribArray(tex_attrib);
    glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0);
    glVertexAttribPointer(tex_attrib, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (GLvoid*)(sizeof(float) * 2));
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(pos_attrib);
    glDisableVertexAttribArray(tex_attrib);
}

void draw_lighted_mesh(mesh* mesh, glm::vec3 const& ambient, glm::vec3 const& diffuse, glm::mat4 const& mv, glm::mat4 const& p) {
    glUseProgram(normal_program);
    
    glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(mv)));
    
    int modelview_uniform = glGetUniformLocation(normal_program, "modelview");
    int projection_uniform = glGetUniformLocation(normal_program, "projection");
    int normal_matrix_uniform = glGetUniformLocation(normal_program, "normal_matrix");
    int light_pos_uniform = glGetUniformLocation(normal_program, "light_pos");
    int diffuse_uniform = glGetUniformLocation(normal_program, "diffuse");
    int ambient_uniform = glGetUniformLocation(normal_program, "ambient");
    int specular_uniform = glGetUniformLocation(normal_program, "specular");
    int shininess_uniform = glGetUniformLocation(normal_program, "shininess");
    
    glUniformMatrix4fv(modelview_uniform, 1, GL_FALSE, &mv[0][0]);
    glUniformMatrix4fv(projection_uniform, 1, GL_FALSE, &p[0][0]);
    glUniformMatrix3fv(normal_matrix_uniform, 1, GL_FALSE, &normal_matrix[0][0]);
    glUniform3f(light_pos_uniform, 0.25f, 0.25f, 1.0f);
    glUniform3f(diffuse_uniform, diffuse.r, diffuse.g, diffuse.b);
    glUniform3f(ambient_uniform, ambient.r, ambient.g, ambient.b);
    glUniform3f(specular_uniform, 0.5f, 0.5f, 0.5f);
    glUniform1f(shininess_uniform, 50.0f);
    
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    
    draw_mesh(mesh, ATTRIB_VERTEX, ATTRIB_NORMAL);  
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glDisableVertexAttribArray(ATTRIB_VERTEX);
    glDisableVertexAttribArray(ATTRIB_NORMAL);
}

void initialize() {    
    attribute thickness_attribs[] = { { ATTRIB_VERTEX, "position" }, { ATTRIB_NORMAL, "normal" } };
    thickness_program = load_shader("thickness.vsh", "thickness.fsh", thickness_attribs, 2);

    attribute sss_attribs[] = { { ATTRIB_VERTEX, "position" }, { ATTRIB_NORMAL, "normal" } };
    sss_program = load_shader("sss.vsh", "sss.fsh", sss_attribs, 2);
    
    attribute normal_attribs[] = { { ATTRIB_VERTEX, "position" }, { ATTRIB_NORMAL, "normal" } };
    normal_program = load_shader("normal.vsh", "normal.fsh", normal_attribs, 2);
    
    create_fbo(scene_fbo, scene_texture, GL_RGB32F_ARB, GL_RGB);
    quad_vbo = create_quad(-1.0f, -1.0f, 1.0f, 1.0f);
    dragon_mesh = load_mesh_ctm("dragon.ctm");      
    buddha_mesh = load_mesh_ctm("buddha.ctm");
    sphere_mesh = load_mesh_ctm("sphere.ctm");
    
    obj_position = glm::vec3(-0.6f, -0.2f, -2.3f);
    light_position = glm::vec3(-0.6f, 0.3f, -2.7f);
    light_translation = glm::vec3(0.0, 0.0025f, 0.0f);
}

void draw() {
    float x = 0.6f;
    float y = x * WINDOW_HEIGHT / WINDOW_WIDTH;
    glm::mat4 perspective = glm::frustum(-x, x, -y, y, 1.0f, 100.0f);
    glm::mat4 translation1 = glm::translate(obj_position);
    glm::mat4 rot = glm::rotate(obj_rotation, 0.0f, 1.0f, 0.0f);
    rot = glm::rotate(rot, 270.0f, 2.0f, 0.0f, 0.0f);
    
    // Draw thicknss into scene_texture
    {
        glBindFramebuffer(GL_FRAMEBUFFER, scene_fbo);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        
        glUseProgram(thickness_program);
        int modelview_uniform = glGetUniformLocation(thickness_program, "modelview");
        int projection_uniform = glGetUniformLocation(thickness_program, "projection");
        
        glm::mat4 modelview = translation1 * rot;
        glUniformMatrix4fv(modelview_uniform, 1, GL_FALSE, &modelview[0][0]);
        glUniformMatrix4fv(projection_uniform, 1, GL_FALSE, &perspective[0][0]);
        draw_mesh(buddha_mesh, ATTRIB_VERTEX, ATTRIB_NORMAL);
        
        glDisable(GL_BLEND);
        //save_framebuffer("out.raw");
    }
    
    // Second pass to perform sub-surface scattering 
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // 0 is the default buffer
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        //glClearColor(0.9f, 0.9f, 0.9f, 0.9f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glm::vec4 transformed_light_pos(0.0f, 0.0f, 0.0f, 1.0f);
        // Draw the light
        {
            glm::mat4 model = glm::translate(light_position);
            model = glm::translate(model, obj_position - light_position);
            model = model * glm::rotate(light_rotation, 0.0f, 1.0f, 0.0f);
            model = glm::translate(model, -(obj_position - light_position));
            transformed_light_pos = model * transformed_light_pos;
            draw_lighted_mesh(sphere_mesh, glm::vec3(1.0f), glm::vec3(0.0f), model, perspective);
        }
        
        glUseProgram(sss_program);
        int modelview_uniform = glGetUniformLocation(sss_program, "modelview");
        int projection_uniform = glGetUniformLocation(sss_program, "projection");
        int light_pos_uniform = glGetUniformLocation(sss_program, "light_pos");
        int thickness_texture_uniform = glGetUniformLocation(sss_program, "thickness_texture");
        int mat_diffuse_uniform = glGetUniformLocation(sss_program, "mat_diffuse");
        int light_diffuse_uniform = glGetUniformLocation(sss_program, "light_diffuse");
        int ambient_color_uniform = glGetUniformLocation(sss_program, "ambient_color");
        int specular_uniform = glGetUniformLocation(sss_program, "specular");
        int shininess_uniform = glGetUniformLocation(sss_program, "shininess");
        int ambient_uniform = glGetUniformLocation(sss_program, "ambient");
        int power_uniform = glGetUniformLocation(sss_program, "power");
        int distortion_uniform = glGetUniformLocation(sss_program, "distortion");
        int scale_uniform = glGetUniformLocation(sss_program, "scale");
        int screen_size_uniform = glGetUniformLocation(sss_program, "screen_size");
        int light_radius_uniform = glGetUniformLocation(sss_program, "light_radius");
        
        glm::mat4 modelview = translation1 * rot;
        glUniformMatrix4fv(modelview_uniform, 1, GL_FALSE, &modelview[0][0]);
        glUniformMatrix4fv(projection_uniform, 1, GL_FALSE, &perspective[0][0]);
        glUniform4f(light_pos_uniform, transformed_light_pos.x, transformed_light_pos.y, transformed_light_pos.z, 1.0f); 
        float factor = 1.0f;
        //glUniform4f(mat_diffuse_uniform, 0.639f * factor, 0.498f * factor, 0.341f * factor, 1.0f);
        glUniform4f(mat_diffuse_uniform, 0.0f, 0.45f, 0.75f, 1.0f);
        glUniform4f(light_diffuse_uniform, 1.0f, 1.0f, 1.0f, 1.0f);
        //glUniform4f(light_diffuse_uniform, 0.3f, 0.8f, 0.2f, 1.0f);
        glUniform3f(ambient_color_uniform, 0.04f, 0.04f, 0.04f);
        glUniform3f(specular_uniform, 0.0f, 0.0f, 0.0f); // (0.5, 0.5, 0.5)
        glUniform1f(shininess_uniform, 200.0f);
        glUniform1f(ambient_uniform, 2.0f); // Play around with this
        glUniform1i(power_uniform, 4.0f);
        glUniform1f(distortion_uniform, 0.2f);
        glUniform1f(scale_uniform, 15.0f);
        glUniform2f(screen_size_uniform, WINDOW_WIDTH, WINDOW_HEIGHT);
        glUniform1f(light_radius_uniform, 1.0f);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, scene_texture);
        glUniform1i(thickness_texture_uniform, 0);
        
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        draw_mesh(buddha_mesh, ATTRIB_VERTEX, ATTRIB_NORMAL);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_DEPTH_TEST);         
    }
}

const char* PezInitialize(int width, int height) {
    initialize();
    return "Sub-surface Scattering";
}

void PezRender() {
    draw();
}

void PezUpdate(unsigned int milliseconds) { }

void PezHandleMouse(int x, int y, int action) {
    static int lastx = 0.0f;
    int delta = x - lastx;
    obj_rotation += delta;
    lastx = x;
}

void PezHandleKeyDown(int key) {
    static float const kTranslation = 0.025f;
    if (key == KEY_UPARROW) {
        light_position.y += kTranslation;
    }
    else if (key == KEY_DOWNARROW) {
        light_position.y -= kTranslation;
    }
    else if (key == KEY_LEFTARROW) {
        light_position.x -= kTranslation;
    }
    else if (key == KEY_RIGHTARROW) {
        light_position.x += kTranslation;
    }
    
}

void PezHandleKeyUp(int key) { }

