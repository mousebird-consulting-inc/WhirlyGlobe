/*
** License Applicability. Except to the extent portions of this file are
** made subject to an alternative license as permitted in the SGI Free
** Software License B, Version 1.1 (the "License"), the contents of this
** file are subject only to the provisions of the License. You may not use
** this file except in compliance with the License. You may obtain a copy
** of the License at Silicon Graphics, Inc., attn: Legal Services, 1600
** Amphitheatre Parkway, Mountain View, CA 94043-1351, or at:
**
** http://oss.sgi.com/projects/FreeB
**
** Note that, as provided in the License, the Software is distributed on an
** "AS IS" basis, with ALL EXPRESS AND IMPLIED WARRANTIES AND CONDITIONS
** DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY IMPLIED WARRANTIES AND
** CONDITIONS OF MERCHANTABILITY, SATISFACTORY QUALITY, FITNESS FOR A
** PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
**
** Original Code. The Original Code is: OpenGL Sample Implementation,
** Version 1.2.1, released January 26, 2000, developed by Silicon Graphics,
** Inc. The Original Code is Copyright (c) 1991-2000 Silicon Graphics, Inc.
** Copyright in any portions created by third parties is as indicated
** elsewhere herein. All Rights Reserved.
**
** Additional Notice Provisions: The application programming interfaces
** established by SGI in conjunction with the Original Code are The
** OpenGL(R) Graphics System: A Specification (Version 1.2.1), released
** April 1, 1999; The OpenGL(R) Graphics System Utility Library (Version
** 1.3), released November 4, 1998; and OpenGL(R) Graphics with the X
** Window System(R) (Version 1.3), released October 19, 1998. This software
** was created using the OpenGL(R) version 1.2.1 Sample Implementation
** published by SGI, but has not been independently verified as being
** compliant with the OpenGL(R) version 1.2.1 Specification.
*/
/*
 *
 * OpenGL ES 1.0 CM port of GLU by Mike Gorchak <mike@malva.ua>
 */

/* Polynomial Evaluator Interface */

#include "glues.h"
#include "gles_evaluator.h"
#include "glimports.h"
#include "glrenderer.h"
#include "glcurveval.h"
#include "nurbsconsts.h"

OpenGLCurveEvaluator::OpenGLCurveEvaluator(void)
{
   // no default callback functions
   beginCallBackN=NULL;
   endCallBackN=NULL;
   vertexCallBackN=NULL;
   normalCallBackN=NULL;
   colorCallBackN=NULL;
   texcoordCallBackN=NULL;
   beginCallBackData=NULL;
   endCallBackData=NULL;
   vertexCallBackData=NULL;
   normalCallBackData=NULL;
   colorCallBackData=NULL;
   texcoordCallBackData=NULL;

   userData=NULL;

   vertex_flag=0;
   normal_flag=0;
   color_flag=0;
   texcoord_flag=0;

   em_vertex.uprime=-1.0;
   em_normal.uprime=-1.0;
   em_color.uprime=-1.0;
   em_texcoord.uprime=-1.0;

   // don't output triangles by default
   output_triangles=0;
}

OpenGLCurveEvaluator::~OpenGLCurveEvaluator(void) 
{
}

/* added nonsense to avoid the warning messages at compile time */
void OpenGLCurveEvaluator::addMap(CurveMap* m)
{
   m=m;
}

void OpenGLCurveEvaluator::range1f(long type, REAL* from, REAL* to)
{
   type=type;
   from=from;
   to=to;
}

void OpenGLCurveEvaluator::domain1f(REAL ulo, REAL uhi)
{
   ulo=ulo;
   uhi=uhi;
}

void OpenGLCurveEvaluator::bgnline(void)
{
   if (output_triangles)
   {
      beginCallBack(GL_LINE_STRIP, userData);
   }
}

void OpenGLCurveEvaluator::endline(void)
{
   if (output_triangles)
   {
      endCallBack(userData);
   }
}

/*---------------------------------------------------------------------------
 * disable - turn off a curve map
 *---------------------------------------------------------------------------
 */
void OpenGLCurveEvaluator::disable(long type)
{
   gluDisable((GLenum)type);
}

/*---------------------------------------------------------------------------
 * enable - turn on a curve map
 *---------------------------------------------------------------------------
 */
void OpenGLCurveEvaluator::enable(long type)
{
   gluEnable((GLenum)type);
}

/*-------------------------------------------------------------------------
 * mapgrid1f - define a lattice of points with origin and offset
 *-------------------------------------------------------------------------
 */
void OpenGLCurveEvaluator::mapgrid1f(long nu, REAL u0, REAL u1)
{
   if (output_triangles)
   {
      global_grid_u0=u0;
      global_grid_u1=u1;
      global_grid_nu=(int)nu;
   }
   else
   {
      global_grid_u0=u0;
      global_grid_u1=u1;
      global_grid_nu=(int)nu;
   }
}

/*-------------------------------------------------------------------------
 * bgnmap1 - preamble to curve definition and evaluations
 *-------------------------------------------------------------------------
 */
void OpenGLCurveEvaluator::bgnmap1f(long)
{
   if (output_triangles)
   {
      // initialized so that no maps are set initially
      vertex_flag=0;
      normal_flag=0;
      color_flag=0;
      texcoord_flag=0;
   }
}

/*-------------------------------------------------------------------------
 * endmap1 - postamble to a curve map
 *-------------------------------------------------------------------------
 */
void OpenGLCurveEvaluator::endmap1f(void)
{
   if (output_triangles)
   {
   }
}

/*-------------------------------------------------------------------------
 * map1f - pass a desription of a curve map
 *-------------------------------------------------------------------------
 */
void OpenGLCurveEvaluator::map1f(long  type,        /* map type */
                                 REAL  ulo,         /* lower parametric bound */
                                 REAL  uhi,         /* upper parametric bound */
                                 long  stride,      /* distance to next point in REALS */
                                 long  order,       /* parametric order */
                                 REAL* pts)         /* control points */
{
   if (output_triangles)
   {
      int dimension=0;
      int which=0;

      switch(type)
      {
         case GLU_MAP1_VERTEX_3:
              which=0;
              dimension=3;
              break;
         case GLU_MAP1_VERTEX_4:
              which=0;
              dimension=4;
              break;
         case GLU_MAP1_INDEX:
              which=2;
              dimension=1;
              break;
         case GLU_MAP1_COLOR_4:
              which=2;
              dimension=4;
              break;
         case GLU_MAP1_NORMAL:
              which=1;
              dimension=3;
              break;
         case GLU_MAP1_TEXTURE_COORD_1:
              which=3;
              dimension=1;
              break;
         case GLU_MAP1_TEXTURE_COORD_2:
              which=3;
              dimension=2;
              break;
         case GLU_MAP1_TEXTURE_COORD_3:
              which=3;
              dimension=3;
              break;
         case GLU_MAP1_TEXTURE_COORD_4:
              which=3;
              dimension=4;
              break;
      }
      inMap1f(which, dimension, ulo, uhi, stride, order, pts);
   }
   else
   {
      int dimension=0;
      int which=0;

      switch(type)
      {
         case GLU_MAP1_VERTEX_3:
              which=0;
              dimension=3;
              break;
         case GLU_MAP1_VERTEX_4:
              which=0;
              dimension=4;
              break;
         case GLU_MAP1_INDEX:
              which=2;
              dimension=1;
              break;
         case GLU_MAP1_COLOR_4:
              which=2;
              dimension=4;
              break;
         case GLU_MAP1_NORMAL:
              which=1;
              dimension=3;
              break;
         case GLU_MAP1_TEXTURE_COORD_1:
              which=3;
              dimension=1;
              break;
         case GLU_MAP1_TEXTURE_COORD_2:
              which=3;
              dimension=2;
              break;
         case GLU_MAP1_TEXTURE_COORD_3:
              which=3;
              dimension=3;
              break;
         case GLU_MAP1_TEXTURE_COORD_4:
              which=3;
              dimension=4;
              break;
      }
      inMap1fr(which, dimension, ulo, uhi, stride, order, pts);
   }
}

/*-------------------------------------------------------------------------
 * mapmesh1f - evaluate a mesh of points on lattice
 *-------------------------------------------------------------------------
 */
void OpenGLCurveEvaluator::mapmesh1f(long style, long from, long to)
{
   output_style=style;

   if (output_triangles)
   {
      inMapMesh1f((int)from, (int)to);
   }
   else
   {
      inMapMesh1fr((int)from, (int)to);
   }
}

/*-------------------------------------------------------------------------
 * evalpoint1i - evaluate a point on a curve
 *-------------------------------------------------------------------------
 */
void OpenGLCurveEvaluator::evalpoint1i(long i)
{
}

/*-------------------------------------------------------------------------
 * evalcoord1f - evaluate a point on a curve
 *-------------------------------------------------------------------------
 */
void OpenGLCurveEvaluator::evalcoord1f(long, REAL u)
{
}

void
#ifdef _WIN32
OpenGLCurveEvaluator::putCallBack(GLenum which, void (APIENTRY *fn)())
#else
OpenGLCurveEvaluator::putCallBack(GLenum which, _GLUfuncptr fn)
#endif
{
   switch (which)
   {
      case GLU_NURBS_BEGIN:
           beginCallBackN=(void (APIENTRY*)(GLenum))fn;
           break;
      case GLU_NURBS_END:
           endCallBackN=(void (APIENTRY*)(void))fn;
           break;
      case GLU_NURBS_VERTEX:
           vertexCallBackN=(void (APIENTRY*)(const GLfloat*))fn;
           break;
      case GLU_NURBS_NORMAL:
           normalCallBackN=(void (APIENTRY*)(const GLfloat*))fn;
           break;
      case GLU_NURBS_COLOR:
           colorCallBackN=(void (APIENTRY *)(const GLfloat*))fn;
           break;
      case GLU_NURBS_TEXTURE_COORD:
           texcoordCallBackN=(void (APIENTRY*)(const GLfloat*))fn;
           break;
      case GLU_NURBS_BEGIN_DATA:
           beginCallBackData=(void (APIENTRY*)(GLenum, void*))fn;
           break;
      case GLU_NURBS_END_DATA:
           endCallBackData=(void (APIENTRY*)(void*))fn;
           break;
      case GLU_NURBS_VERTEX_DATA:
           vertexCallBackData=(void (APIENTRY*)(const GLfloat*, void*))fn;
           break;
      case GLU_NURBS_NORMAL_DATA:
           normalCallBackData=(void (APIENTRY*)(const GLfloat*, void*))fn;
           break;
      case GLU_NURBS_COLOR_DATA:
           colorCallBackData=(void (APIENTRY*)(const GLfloat*, void*))fn;
           break;
      case GLU_NURBS_TEXTURE_COORD_DATA:
           texcoordCallBackData=(void (APIENTRY*)(const GLfloat*, void*))fn;
           break;
   }
}

void OpenGLCurveEvaluator::beginCallBack(GLenum which, void* data)
{
   if (beginCallBackData)
   {
      beginCallBackData(which, data);
   }
   else
   {
      if (beginCallBackN)
      {
         beginCallBackN(which);
      }
   }
}

void OpenGLCurveEvaluator::endCallBack(void* data)
{
   if (endCallBackData)
   {
      endCallBackData(data);
   }
   else
   {
      if (endCallBackN)
      {
         endCallBackN();
      }
   }
}

void OpenGLCurveEvaluator::vertexCallBack(const GLfloat* vert, void* data)
{
   if (vertexCallBackData)
   {
      vertexCallBackData(vert, data);
   }
   else
   {
      if (vertexCallBackN)
      {
         vertexCallBackN(vert);
      }
   }
}

void OpenGLCurveEvaluator::normalCallBack(const GLfloat* normal, void* data)
{
   if (normalCallBackData)
   {
      normalCallBackData(normal, data);
   }
   else
   {
      if (normalCallBackN)
      {
         normalCallBackN(normal);
      }
   }
}

void OpenGLCurveEvaluator::colorCallBack(const GLfloat* color, void* data)
{
   if (colorCallBackData)
   {
      colorCallBackData(color, data);
   }
   else
   {
      if (colorCallBackN)
      {
         colorCallBackN(color);
      }
   }
}

void OpenGLCurveEvaluator::texcoordCallBack(const GLfloat* texcoord, void* data)
{
   if (texcoordCallBackData)
   {
      texcoordCallBackData(texcoord, data);
   }
   else
   {
      if (texcoordCallBackN)
      {
         texcoordCallBackN(texcoord);
      }
   }
}
