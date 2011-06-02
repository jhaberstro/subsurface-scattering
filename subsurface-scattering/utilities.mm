//
//  path_utilities.cpp
//  sub-surface scattering
//
//  Created by Jedd Haberstro on 5/9/11.
//  Copyright 2011 Student. All rights reserved.
//

#include "utilities.h"
#include "pez/pez.h"
#import <Cocoa/Cocoa.h>
#include <cstdio>
#include <TargetConditionals.h>

namespace
{
    char* read_shader_source(char const* filepath, GLint& outSize) {
        FILE* file = std::fopen(filepath, "r");
        assert(file != 0);
        
        std::fseek(file, 0, SEEK_END);
        unsigned long size = (unsigned long)ftell(file);
        std::rewind(file);
        
        char* contents = new char[size];
        size_t result = std::fread(contents, 1, size, file);
        assert(result == size);
        
        std::fclose(file);
        outSize = (GLint)size;
        return contents;
    }
    
    bool check_compiled_status(GLuint handle) {
        GLint status = GL_TRUE;
        glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE) {
            GLint infoLogLength = 0;
            glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &infoLogLength);
            char infoLog[infoLogLength];
            glGetShaderInfoLog(handle, infoLogLength, 0, &infoLog[0]);
            printf("%s\n", infoLog);
        }
        
        return status == GL_TRUE;
    }
}

char const* path_for_resource_and_type(char const* resource, char const* type) {
    NSString* resourceStr = [[NSString alloc] initWithUTF8String:resource];
    NSString* typeStr = [[NSString alloc] initWithUTF8String:type];
    return [[[NSBundle mainBundle] pathForResource:resourceStr ofType:typeStr] UTF8String];
}

char const* path_for_resource_and_type(char const* file) {
    NSString* basePath = [NSString stringWithUTF8String:file];
    NSString* resourcePath = [[NSBundle mainBundle] resourcePath];
    NSString* path = [resourcePath stringByAppendingPathComponent:basePath];
    return [path UTF8String];
}

void load_png(char const* file)
{
    NSString* basePath = [NSString stringWithUTF8String:file];
    NSString* resourcePath = [[NSBundle mainBundle] resourcePath];
    NSString* path = [resourcePath stringByAppendingPathComponent:basePath];    
#if TARGET_OS_IPHONE
    NSData *texData = [[NSData alloc] initWithContentsOfFile:path];
    UIImage *image = [[UIImage alloc] initWithData:texData];
    [texData release];
#else
    NSImage *nsimage = [[NSImage alloc] initWithContentsOfFile: path];
	NSBitmapImageRep *image = [[NSBitmapImageRep alloc] initWithData:[nsimage TIFFRepresentation]];
	[nsimage release];
#endif
    assert(image != nil);
    
    GLuint width = CGImageGetWidth(image.CGImage);
    GLuint height = CGImageGetHeight(image.CGImage);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    void *imageData = malloc( height * width * 4 );
    CGContextRef context = CGBitmapContextCreate( imageData, width, height, 8, 4 * width, colorSpace, kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big );
    CGColorSpaceRelease( colorSpace );
    CGContextClearRect( context, CGRectMake( 0, 0, width, height ) );
    CGContextTranslateCTM( context, 0, height - height );
    CGContextDrawImage( context, CGRectMake( 0, 0, width, height ), image.CGImage );
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
    
    CGContextRelease(context);
    
    free(imageData);
    [image release];
}

void freeImageData(void *info, const void *data, size_t size) 
{
    //printf("freeImageData called");
    free((void*)data);
}

void save_framebuffer(char const* filename) {
    GLubyte *buffer = (GLubyte *) malloc(PEZ_VIEWPORT_WIDTH * PEZ_VIEWPORT_HEIGHT * 4);
    GLubyte *buffer2 = (GLubyte *) malloc(PEZ_VIEWPORT_WIDTH * PEZ_VIEWPORT_HEIGHT * 4);
    
    GLvoid *pixel_data = nil;
    glReadPixels(0, 0, PEZ_VIEWPORT_WIDTH, PEZ_VIEWPORT_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)buffer);
    
    /* make upside down */
    
    for (int y=0; y<PEZ_VIEWPORT_HEIGHT; y++) {
        for (int x=0; x<PEZ_VIEWPORT_WIDTH*4; x++) {
            buffer2[y * 4 * PEZ_VIEWPORT_WIDTH + x] = buffer[(PEZ_VIEWPORT_HEIGHT - y - 1) * PEZ_VIEWPORT_WIDTH * 4 + x];
        }
    }
    
    // make data provider from buffer
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, buffer2, PEZ_VIEWPORT_WIDTH * PEZ_VIEWPORT_HEIGHT * 4, freeImageData);
    
    // set up for CGImage creation
    int bitsPerComponent = 8;
    int bitsPerPixel = 32;
    int bytesPerRow = 4 * PEZ_VIEWPORT_WIDTH;
    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault;
    // Use this to retain alpha
    //CGBitmapInfo bitmapInfo = kCGImageAlphaPremultipliedLast;
    CGColorRenderingIntent renderingIntent = kCGRenderingIntentDefault;
    CGImageRef imageRef = CGImageCreate(PEZ_VIEWPORT_WIDTH, PEZ_VIEWPORT_HEIGHT, bitsPerComponent, bitsPerPixel, bytesPerRow, colorSpaceRef, bitmapInfo, provider, NULL, NO, renderingIntent);
    
    NSURL *outURL = [[NSURL alloc] initFileURLWithPath:[NSString stringWithUTF8String:filename]]; 
	CGImageDestinationRef dr = CGImageDestinationCreateWithURL ((CFURLRef)outURL, (CFStringRef)@"public.png" , 1, NULL);
	CGImageDestinationAddImage(dr, imageRef, NULL);
	CGImageDestinationFinalize(dr);
}

GLuint load_shader(char const* vsh, char const* fsh, attribute const* attributes, int n) {
    GLuint program = glCreateProgram();
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    
    GLint vs_source_size = 0;
    char const* vs_source = read_shader_source(path_for_resource_and_type(vsh), vs_source_size);
    glShaderSource(vs, 1, &vs_source, &vs_source_size);
    glCompileShader(vs);
    assert(check_compiled_status(vs));
    glAttachShader(program, vs);
    
    GLint fs_source_size = 0;
    char const* fs_source = read_shader_source(path_for_resource_and_type(fsh), fs_source_size);
    glShaderSource(fs, 1, &fs_source, &fs_source_size);
    glCompileShader(fs);
    assert(check_compiled_status(fs));
    glAttachShader(program, fs);
    
    for (int i = 0; i < n; ++i) {
        printf("attribute: (%i, %s)\n", attributes[i].index, attributes[i].name);
        glBindAttribLocation(program, attributes[i].index, attributes[i].name);
    }
    
    GLint linkStatus = GL_TRUE;
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE) {
        GLint infoLogLength = 0;
        glGetShaderiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
        char infoLog[infoLogLength];
        glGetShaderInfoLog(program, infoLogLength, 0, &infoLog[0]);
        printf("%s\n", infoLog);
        assert(false);
    }
    
    return program;
}

