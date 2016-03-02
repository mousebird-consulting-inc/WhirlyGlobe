/*
 * SGI FREE SOFTWARE LICENSE B (Version 2.0, Sept. 18, 2008)
 * Copyright (C) 1991-2000 Silicon Graphics, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice including the dates of first publication and
 * either this permission notice or a reference to
 * http://oss.sgi.com/projects/FreeB/
 * shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * SILICON GRAPHICS, INC. BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of Silicon Graphics, Inc.
 * shall not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization from
 * Silicon Graphics, Inc.
 */

/*
 * gles_evaluator.cc
 * Author: Mike Gorchak, 2009. mike@malva.ua, lestat@i.com.ua
 *
 */

#include "glues.h"
#include "gles_evaluator.h"

#include <memory.h>

#include <stdio.h>

GLAPI void APIENTRY gluEnable(GLenum cap)
{
   switch (cap)
   {
      case GLU_MAP1_COLOR_4:
      case GLU_MAP1_INDEX:
      case GLU_MAP1_NORMAL:
      case GLU_MAP1_TEXTURE_COORD_1:
      case GLU_MAP1_TEXTURE_COORD_2:
      case GLU_MAP1_TEXTURE_COORD_3:
      case GLU_MAP1_TEXTURE_COORD_4:
      case GLU_MAP1_VERTEX_3:
      case GLU_MAP1_VERTEX_4:
      case GLU_MAP2_COLOR_4:
      case GLU_MAP2_INDEX:
      case GLU_MAP2_NORMAL:
      case GLU_MAP2_TEXTURE_COORD_1:
      case GLU_MAP2_TEXTURE_COORD_2:
      case GLU_MAP2_TEXTURE_COORD_3:
      case GLU_MAP2_TEXTURE_COORD_4:
      case GLU_MAP2_VERTEX_3:
      case GLU_MAP2_VERTEX_4:
      case GLU_AUTO_NORMAL:
           break;
      default:
           glEnable(cap);
           break;
   }
}

GLAPI void APIENTRY gluDisable(GLenum cap)
{
   switch (cap)
   {
      case GLU_MAP1_COLOR_4:
      case GLU_MAP1_INDEX:
      case GLU_MAP1_NORMAL:
      case GLU_MAP1_TEXTURE_COORD_1:
      case GLU_MAP1_TEXTURE_COORD_2:
      case GLU_MAP1_TEXTURE_COORD_3:
      case GLU_MAP1_TEXTURE_COORD_4:
      case GLU_MAP1_VERTEX_3:
      case GLU_MAP1_VERTEX_4:
      case GLU_MAP2_COLOR_4:
      case GLU_MAP2_INDEX:
      case GLU_MAP2_NORMAL:
      case GLU_MAP2_TEXTURE_COORD_1:
      case GLU_MAP2_TEXTURE_COORD_2:
      case GLU_MAP2_TEXTURE_COORD_3:
      case GLU_MAP2_TEXTURE_COORD_4:
      case GLU_MAP2_VERTEX_3:
      case GLU_MAP2_VERTEX_4:
      case GLU_AUTO_NORMAL:
           break;
      default:
           glDisable(cap);
           break;
   }
}

GLfloat lg2table[63]=
{
   0.0000000004656612873077392578125f,
   0.000000000931322574615478515625f,
   0.00000000186264514923095703125f,
   0.0000000037252902984619140625f,
   0.000000007450580596923828125f,
   0.00000001490116119384765625f,
   0.0000000298023223876953125f,
   0.000000059604644775390625f,

   0.00000011920928955078125f,
   0.0000002384185791015625f,
   0.000000476837158203125f,
   0.00000095367431640625f,
   0.0000019073486328125f,
   0.000003814697265625f,
   0.00000762939453125f,
   0.0000152587890625f,

   0.000030517578125f,
   0.00006103515625f,
   0.0001220703125f,
   0.000244140625f,
   0.00048828125f,
   0.0009765625f,
   0.001953125f,        /* 1/512 */
   0.00390625f,         /* 1/256 */

   0.0078125f,          /* 1/128 */
   0.015625f,           /* 1/64  */
   0.03125f,            /* 1/32  */
   0.0625f,             /* 1/16  */
   0.125f,              /* 1/8   */
   0.25f,               /* 1/4   */
   0.5f,                /* 1/2   */

   0x00000001,          /* zero */
   0x00000002,
   0x00000004,
   0x00000008,
   0x00000010,
   0x00000020,
   0x00000040,
   0x00000080,

   0x00000100,
   0x00000200,
   0x00000400,
   0x00000800,
   0x00001000,
   0x00002000,
   0x00004000,
   0x00008000,

   0x00010000,
   0x00020000,
   0x00040000,
   0x00080000,
   0x00100000,
   0x00200000,
   0x00400000,
   0x00800000,

   0x01000000,
   0x02000000,
   0x04000000,
   0x08000000,
   0x10000000,
   0x20000000,
   0x40000000,
   0x80000000
};

GLAPI void APIENTRY gluGetFloatv(GLenum pname, GLfloat* params)
{
   switch (pname)
   {
      case GL_MODELVIEW_MATRIX:
      case GL_PROJECTION_MATRIX:
           /* Check if OpenGL ES 1.1 is used, then call glGetFloatv directly */
           #if defined(GL_VERSION_ES_CM_1_1)
              /* Just passthrough the request to OpenGL ES 1.1 */
              glGetFloatv(pname, params);
              return;
           #endif /* GL_VERSION_ES_CM_1_1 */
           /* Check if OpenGL ES 1.0 is used, then try to emulate glGetFloatv */
           #if (defined(GL_OES_VERSION_1_0) || defined(GL_VERSION_ES_CM_1_0)) && !defined(GL_VERSION_ES_CM_1_1)
              /* Check for query_matrix extension, which is very usefull in OpenGL ES 1.0 to obtain */
              /* GLES 1.0 dynamic state                                                             */
              #if defined(GL_OES_query_matrix)
                 {
                    GLfixed mantissa[16];
                    GLint exponent[16];

                    /* Clear the output data, in case if glQueryMatrixxOES() will fail */
                    memset(params, 0x00, 16*sizeof(GLfloat));

                    /* Since OpenGL ES 1.0 has no GL_MATRIX_MODE for glGet() we will try to setup    */
                    /* required matrix and then restore modelview matrix, because it must be default */
                    /* current matrix mode for rendering process.                                    */
                    switch (pname)
                    {
                       case GL_MODELVIEW_MATRIX:
                            glMatrixMode(GL_MODELVIEW);
                            break;
                       case GL_PROJECTION_MATRIX:
                            glMatrixMode(GL_PROJECTION);
                            break;
                    }

                    /* Query current matrix content */
                    if (glQueryMatrixxOES(mantissa, exponent)==0)
                    {
                       for (int it=0; it<16; it++)
                       {
                          params[it]=FX2F(mantissa[it])*lg2table[exponent[it]+31];
                       }
                    }

                    /* Restore "default" matrix mode */
                    glMatrixMode(GL_MODELVIEW);
                 }
              #else
                 #error "Do not know how to query modelview and projection matrices"
              #endif /* GL_OES_query_matrix */
           #endif /* GL_OES_VERSION_1_0 or GL_VERSION_ES_CM_1_0 only */
           break;
   }

   #if defined(GL_VERSION_ES_CM_1_1)
      /* Just passthrough the request to OpenGL ES 1.1             */
      /* In OpenGL ES 1.0 all other Float requests will be ignored */
      glGetFloatv(pname, params);
   #endif /* GL_VERSION_ES_CM_1_1 */
}

GLint glu_viewport[4];

GLAPI void APIENTRY gluGetIntegerv(GLenum pname, GLint* params)
{
   switch (pname)
   {
      case GL_VIEWPORT:
           #if (defined(GL_OES_VERSION_1_0) || defined(GL_VERSION_ES_CM_1_0)) && !defined(GL_VERSION_ES_CM_1_1)
           params[0]=glu_viewport[0];
           params[1]=glu_viewport[1];
           params[2]=glu_viewport[2];
           params[3]=glu_viewport[3];
           return;
           #endif /* GL_OES_VERSION_1_0 or GL_VERSION_ES_CM_1_0 only */
           break;

   }

   glGetIntegerv(pname, params);
}

GLAPI void APIENTRY gluViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
   glu_viewport[0]=x;
   glu_viewport[1]=y;
   glu_viewport[2]=(GLint)width;
   glu_viewport[3]=(GLint)height;
}
