/*
 *  glwrapper.cpp
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

#import <string.h>
#import "glwrapper.h"

#ifdef __ANDROID__

bool hasVertexArraySupport = false;
bool hasMapBufferSupport = false;
bool hasInstanceSupport = false;

// Note: Porting
PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayEXT = NULL;
PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysEXT = NULL;
PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysEXT = NULL;
PFNGLMAPBUFFEROESPROC glMapBufferEXT = NULL;
PFNGLUNMAPBUFFEROESPROC glUnmapBufferEXT = NULL;
//PFNGLVERTEXATTRIBDIVISOREXTPROC glVertexAttribDivisorEXT = NULL;
//PFNGLDRAWELEMENTSINSTANCEDEXTPROC glDrawElementsInstancedEXT = NULL;
//PFNGLDRAWARRAYSINSTANCEDEXTPROC glDrawArraysInstancedEXT = NULL;

// Wire up the various function pointers for extensions
bool SetupGLESExtensions()
{
	const char *cap = (const char *)glGetString(GL_EXTENSIONS);

    // Note: Porting
//	if (strstr(cap,"GL_OES_vertex_array_object"))
//		hasVertexArraySupport = true;
//	if (strstr(cap,"GL_OES_mapbuffer"))
//		hasMapBufferSupport = true;
//
//	if (hasVertexArraySupport)
//	{
//		glBindVertexArrayEXT = (PFNGLBINDVERTEXARRAYOESPROC)eglGetProcAddress ( "glBindVertexArrayOES" );
//		if ( !glBindVertexArrayEXT )
//			return false;
//
//		glDeleteVertexArraysEXT = (PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress ( "glDeleteVertexArraysOES" );
//		if ( !glDeleteVertexArraysEXT )
//			return false;
//
//	    glGenVertexArraysEXT = (PFNGLGENVERTEXARRAYSOESPROC)eglGetProcAddress ( "glGenVertexArraysOES" );
//	    if ( !glGenVertexArraysEXT )
//	        return false;
//	}
//    
//	if (hasMapBufferSupport)
//	{
//		glMapBufferEXT = (PFNGLMAPBUFFEROESPROC)eglGetProcAddress ( "glMapBufferOES" );
//		if ( !glMapBufferEXT )
//			return false;
//
//		glUnmapBufferEXT = (PFNGLUNMAPBUFFEROESPROC)eglGetProcAddress ( "glUnmapBufferOES" );
//		if ( !glUnmapBufferEXT )
//			return false;
//	}
    
    // note: Porting.  Debugging VAO's
    hasVertexArraySupport = false;
    
    return true;
}

// Note: Porting
//GL_APICALL void GL_APIENTRY glBindVertexArrayOES (GLuint array)
//{
//    return (*glBindVertexArrayEXT)(array);
//}
//
//GL_APICALL void GL_APIENTRY glDeleteVertexArraysOES (GLsizei n, const GLuint *arrays)
//{
//    return (*glDeleteVertexArraysEXT)(n,arrays);
//}
//
//GL_APICALL void GL_APIENTRY glGenVertexArraysOES (GLsizei n, GLuint *arrays)
//{
//    return (*glGenVertexArraysEXT)(n,arrays);
//}
//
//GL_APICALL void* GL_APIENTRY glMapBufferOES (GLenum target, GLenum access)
//{
//    return (*glMapBufferEXT)(target,access);
//}
//
//GL_APICALL GLboolean GL_APIENTRY glUnmapBufferOES (GLenum target)
//{
//    return (*glUnmapBufferEXT)(target);
//}

// Note: There are here to make the linker happy.

void glBindVertexArray (GLuint array)
{
    return (*glBindVertexArrayEXT)(array);
}

void glDeleteVertexArrays (GLsizei n, const GLuint *arrays)
{
    return (*glDeleteVertexArraysEXT)(n,arrays);
}

void glGenVertexArrays (GLsizei n, GLuint *arrays)
{
    return (*glGenVertexArraysEXT)(n,arrays);
}

void* glMapBuffer (GLenum target, GLenum access)
{
    return (*glMapBufferEXT)(target,access);
}

GLboolean glUnmapBuffer (GLenum target)
{
    return (*glUnmapBufferEXT)(target);
}

void glVertexAttribDivisor (GLuint index, GLuint divisor)
{
//    glVertexAttribDivisorEXT(index,divisor);
}

void glDrawElementsInstanced(	GLenum mode,
                             GLsizei count,
                             GLenum type,
                             const void * indices,
                             GLsizei primcount)
{
//    return (*glDrawElementsInstancedEXT)(mode,count,type,indices,primcount);
}

void glDrawArraysInstanced(	GLenum mode,
                           GLint first,
                           GLsizei count,
                           GLsizei primcount)
{
//    return (*glDrawArraysInstancedEXT)(mode,first,count,primcount);
}

#else

// On ios we have both
// Note: Debugging
bool hasVertexArraySupport = false;
bool hasMapBufferSupport = false;

#endif

