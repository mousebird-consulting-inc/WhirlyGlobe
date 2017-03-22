/*
 *  glwrapper.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 12/18/13.
 *  Copyright 2011-2013 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#ifdef __ANDROID__ 

#import <jni.h>
#import <android/log.h>

#define GL_GLEXT_PROTOTYPES
//#define __USE_SDL_GLES__
#include <GLES2/gl2platform.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
// Note: Would be nice to make this conditional
//#include <GLES3/gl3platform.h>
//#include <GLES3/gl3.h>
//#include <GLES3/gl3ext.h>
#include <EGL/egl.h>

/// Returns false if it can't find all the extensions it needs
extern bool SetupGLESExtensions();

// Note: Here for compatibility.
void glBindVertexArray (GLuint array);
void glDeleteVertexArrays (GLsizei n, const GLuint *arrays);
void glGenVertexArrays (GLsizei n, GLuint *arrays);
void* glMapBuffer (GLenum target, GLenum access);
GLboolean glUnmapBuffer (GLenum target);
void glVertexAttribDivisor (GLuint index, GLuint divisor);
void glDrawElementsInstanced(	GLenum mode,
                             GLsizei count,
                             GLenum type,
                             const void * indices,
                             GLsizei primcount);
void glDrawArraysInstanced(	GLenum mode,
                           GLint first,
                           GLsizei count,
                           GLsizei primcount);

#else

// iOS
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>

#endif

extern bool hasVertexArraySupport;
extern bool hasMapBufferSupport;
