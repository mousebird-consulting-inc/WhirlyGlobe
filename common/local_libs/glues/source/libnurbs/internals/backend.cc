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

/* Bezier surface backend - interprets display mode (wireframe, shaded, ...) */
#include <stdio.h>
#include "glimports.h"
#include "mystdio.h"
#include "backend.h"
#include "basiccrveval.h"
#include "basicsurfeval.h"
#include "nurbsconsts.h"

#include "gles_evaluator.h"
#include "glues.h"

/*-------------------------------------------------------------------------
 * bgnsurf - preamble to surface definition and evaluations
 *-------------------------------------------------------------------------
 */
void Backend::bgnsurf(int wiretris, int wirequads, long nuid)
{
   wireframetris=wiretris;
   wireframequads=wirequads;

   /* in the spec, GLU_DISPLAY_MODE is either
    * GLU_FILL
    * GLU_OUTLINE_POLY
    * GLU_OUTLINE_PATCH.
    * In fact, GLU_FILL has the same effect as
    * set GL_FRONT_AND_BACK to be GL_FILL
    * and GLU_OUTLINE_POLY is the same as set 
    *     GL_FRONT_AND_BACK to be GL_LINE
    * It is more efficient to do this once at the beginning of
    * each surface than to do it for each primitive.
    *   The internal has more options: outline_triangle and outline_quad
    * can be seperated. But since this is not in spec, and more importantly,
    * this is not so useful, so we don't need to keep this option.
    */

   surfaceEvaluator.bgnmap2f(nuid);

   if (wiretris)
   {
      surfaceEvaluator.polymode(N_MESHLINE);
   }
   else
   {
      surfaceEvaluator.polymode(N_MESHFILL);
   }
}

void Backend::patch(REAL ulo, REAL uhi, REAL vlo, REAL vhi)
{
   surfaceEvaluator.domain2f(ulo, uhi, vlo, vhi);
}

void Backend::surfbbox(long type, REAL* from, REAL* to)
{
   surfaceEvaluator.range2f(type, from, to);
}

/*-------------------------------------------------------------------------
 * surfpts - pass a desription of a surface map
 *-------------------------------------------------------------------------
 */
void Backend::surfpts(long type,        /* geometry, color, texture, normal      */
                      REAL* pts,        /* control points                        */
                      long ustride,     /* distance to next point in u direction */
                      long vstride,     /* distance to next point in v direction */
                      int uorder,       /* u parametric order                    */
                      int vorder,       /* v parametric order                    */
                      REAL ulo,         /* u lower bound                         */
                      REAL uhi,         /* u upper bound                         */
                      REAL vlo,         /* v lower bound                         */
                      REAL vhi)         /* v upper bound                         */
{
   surfaceEvaluator.map2f(type, ulo, uhi, ustride, uorder, vlo, vhi, vstride, vorder, pts);
   surfaceEvaluator.enable(type);
}

/*-------------------------------------------------------------------------
 * surfgrid - define a lattice of points with origin and offset
 *-------------------------------------------------------------------------
 */
void Backend::surfgrid(REAL u0, REAL u1, long nu, REAL v0, REAL v1, long nv)
{
   surfaceEvaluator.mapgrid2f(nu, u0, u1, nv, v0, v1);
}

/*-------------------------------------------------------------------------
 * surfmesh - evaluate a mesh of points on lattice
 *-------------------------------------------------------------------------
 */
void Backend::surfmesh(long u, long v, long n, long m)
{
   if (wireframequads)
   {
      surfaceEvaluator.mapmesh2f(N_MESHLINE, u, u+n, v, v+m);
   }
   else
   {
      surfaceEvaluator.mapmesh2f(N_MESHFILL, u, u+n, v, v+m);
   }
}

/*-------------------------------------------------------------------------
 * endsurf - postamble to surface
 *-------------------------------------------------------------------------
 */
void Backend::endsurf(void)
{
   surfaceEvaluator.endmap2f();
}

/***************************************/
void Backend::bgntfan(void)
{
   surfaceEvaluator.bgntfan();
}

void Backend::endtfan(void)
{
   surfaceEvaluator.endtfan();
}

void Backend::bgnqstrip(void)
{
   surfaceEvaluator.bgnqstrip();
}

void Backend::endqstrip(void)
{
   surfaceEvaluator.endqstrip();
}

void Backend::evalUStrip(int n_upper, REAL v_upper, REAL* upper_val,
                         int n_lower, REAL v_lower, REAL* lower_val)
{
   surfaceEvaluator.evalUStrip(n_upper, v_upper, upper_val, n_lower, v_lower, lower_val);
}

void Backend::evalVStrip(int n_left, REAL u_left, REAL* left_val,
                         int n_right, REAL u_right, REAL* right_val)
{
   surfaceEvaluator.evalVStrip(n_left, u_left, left_val, n_right, u_right, right_val);
}

/*-------------------------------------------------------------------------
 * bgntmesh - preamble to a triangle mesh
 *-------------------------------------------------------------------------
 */
void Backend::bgntmesh(const char* name)
{
   if (wireframetris)
   {
      surfaceEvaluator.bgntmesh();
      surfaceEvaluator.polymode(N_MESHLINE);
   }
   else
   {
      surfaceEvaluator.bgntmesh();
      surfaceEvaluator.polymode(N_MESHFILL);
    }
}

void Backend::tmeshvert(GridTrimVertex* v)
{
   REAL retPoint[4];
   REAL retNormal[3];

   if (v->isGridVert())
   {
      tmeshvert(v->g);
   }
   else
   {
      tmeshvert(v->t, retPoint, retNormal);
   }
}

void Backend::tmeshvertNOGE(TrimVertex* t)
{
}

// opt for a line with the same u.
void Backend::tmeshvertNOGE_BU(TrimVertex* t)
{
}

// opt for a line with the same v.
void Backend::tmeshvertNOGE_BV(TrimVertex* t)
{
}

void Backend::preEvaluateBU(REAL u)
{
   surfaceEvaluator.inPreEvaluateBU_intfac(u);
}

void Backend::preEvaluateBV(REAL v)
{
   surfaceEvaluator.inPreEvaluateBV_intfac(v);
}

/*-------------------------------------------------------------------------
 * tmeshvert - evaluate a point on a triangle mesh
 *-------------------------------------------------------------------------
 */
void Backend::tmeshvert(TrimVertex* t, REAL* retPoint, REAL* retNormal)
{
   const REAL u=t->param[0];
   const REAL v=t->param[1];

   surfaceEvaluator.evalcoord2f(0, u, v, retPoint, retNormal);
}

// the same as tmeshvert(trimvertex), for efficiency purpose
void Backend::tmeshvert(REAL u, REAL v, REAL* retPoint, REAL* retNormal)
{
   surfaceEvaluator.evalcoord2f(0, u, v, retPoint, retNormal);
}

/*-------------------------------------------------------------------------
 * tmeshvert - evaluate a grid point of a triangle mesh
 *-------------------------------------------------------------------------
 */
void Backend::tmeshvert(GridVertex* g)
{
   const long u=g->gparam[0];
   const long v=g->gparam[1];

   surfaceEvaluator.evalpoint2i(u, v);
}

/*-------------------------------------------------------------------------
 * swaptmesh - perform a swap of the triangle mesh pointers
 *-------------------------------------------------------------------------
 */
void Backend::swaptmesh(void)
{
   surfaceEvaluator.swaptmesh();
}

/*-------------------------------------------------------------------------
 * endtmesh - postamble to triangle mesh
 *-------------------------------------------------------------------------
 */
void Backend::endtmesh(void)
{
   surfaceEvaluator.endtmesh();
}

/*-------------------------------------------------------------------------
 * bgnoutline - preamble to outlined rendering
 *-------------------------------------------------------------------------
 */
void Backend::bgnoutline(void)
{
   surfaceEvaluator.bgnline();
}

/*-------------------------------------------------------------------------
 * linevert - evaluate a point on an outlined contour
 *-------------------------------------------------------------------------
 */
void Backend::linevert(TrimVertex* t, REAL* retPoint, REAL* retNormal)
{
   surfaceEvaluator.evalcoord2f(t->nuid, t->param[0], t->param[1], retPoint, retNormal);
}

/*-------------------------------------------------------------------------
 * linevert - evaluate a grid point of an outlined contour
 *-------------------------------------------------------------------------
 */
void Backend::linevert(GridVertex* g)
{
   surfaceEvaluator.evalpoint2i(g->gparam[0], g->gparam[1]);
}

/*-------------------------------------------------------------------------
 * endoutline - postamble to outlined rendering
 *-------------------------------------------------------------------------
 */
void Backend::endoutline(void)
{
   surfaceEvaluator.endline();
}

/*-------------------------------------------------------------------------
 * triangle - output a triangle
 *-------------------------------------------------------------------------
 */
void Backend::triangle(TrimVertex* a, TrimVertex* b, TrimVertex* c)
{
   REAL retPoint[4];
   REAL retNormal[3];
   REAL vertices[3*3];
   REAL normals[3*3];

   GLboolean texcoord_enabled;
   GLboolean normal_enabled;
   GLboolean vertex_enabled;
   GLboolean color_enabled;

   /* Store status of enabled arrays */
   texcoord_enabled=GL_FALSE; /* glIsEnabled(GL_TEXTURE_COORD_ARRAY); */
   normal_enabled=GL_FALSE;   /* glIsEnabled(GL_NORMAL_ARRAY);        */
   vertex_enabled=GL_FALSE;   /* glIsEnabled(GL_VERTEX_ARRAY);        */
   color_enabled=GL_FALSE;    /* glIsEnabled(GL_COLOR_ARRAY);         */

   /* Enable needed and disable unneeded arrays */
   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(3, GL_FLOAT, 0, vertices);
   glEnableClientState(GL_NORMAL_ARRAY);
   glNormalPointer(GL_FLOAT, 0, normals);
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   glDisableClientState(GL_COLOR_ARRAY);

   bgntfan();
   tmeshvert(a, retPoint, retNormal);
   vertices[0]=retPoint[0];
   vertices[1]=retPoint[1];
   vertices[2]=retPoint[2];
   normals[0]=retNormal[0];
   normals[1]=retNormal[1];
   normals[2]=retNormal[2];
   tmeshvert(b, retPoint, retNormal);
   vertices[3]=retPoint[0];
   vertices[4]=retPoint[1];
   vertices[5]=retPoint[2];
   normals[3]=retNormal[0];
   normals[4]=retNormal[1];
   normals[5]=retNormal[2];
   tmeshvert(c, retPoint, retNormal);
   vertices[6]=retPoint[0];
   vertices[7]=retPoint[1];
   vertices[8]=retPoint[2];
   normals[6]=retNormal[0];
   normals[7]=retNormal[1];
   normals[8]=retNormal[2];
   endtfan();

   if (get_output_style()==N_MESHLINE)
   {
      glDrawArrays(GL_LINE_LOOP, 0, 3);
   }
   else
   {
      glDrawArrays(GL_TRIANGLE_FAN, 0, 3);
   }

   /* Disable or re-enable arrays */
   if (vertex_enabled)
   {
      /* Re-enable vertex array */
      glEnableClientState(GL_VERTEX_ARRAY);
   }
   else
   {
      glDisableClientState(GL_VERTEX_ARRAY);
   }

   if (texcoord_enabled)
   {
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   }
   else
   {
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   }

   if (normal_enabled)
   {
      glEnableClientState(GL_NORMAL_ARRAY);
   }
   else
   {
      glDisableClientState(GL_NORMAL_ARRAY);
   }

   if (color_enabled)
   {
      glEnableClientState(GL_COLOR_ARRAY);
   }
   else
   {
      glDisableClientState(GL_COLOR_ARRAY);
   }
}

long Backend::get_output_style(void)
{
   return surfaceEvaluator.get_output_style();
}

void Backend::bgncurv(void)
{
   curveEvaluator.bgnmap1f(0);
}

void Backend::segment(REAL ulo, REAL uhi)
{
   curveEvaluator.domain1f(ulo, uhi);
}

void Backend::curvpts(long type,        /* geometry, color, texture, normal */
                      REAL* pts,        /* control points */
                      long stride,      /* distance to next point */
                      int order,        /* parametric order */
                      REAL ulo,         /* lower parametric bound */
                      REAL uhi)         /* upper parametric bound */

{
   curveEvaluator.map1f(type, ulo, uhi, stride, order, pts);
   curveEvaluator.enable(type);
}

void Backend::curvgrid(REAL u0, REAL u1, long nu)
{
   curveEvaluator.mapgrid1f(nu, u0, u1);
}

void Backend::curvmesh(long from, long n)
{
   curveEvaluator.mapmesh1f(N_MESHFILL, from, from+n);
}

void Backend::curvpt(REAL u)
{
   curveEvaluator.evalcoord1f(0, u);
}

void Backend::bgnline(void)
{
   curveEvaluator.bgnline();
}

void Backend::endline(void)
{
   curveEvaluator.endline();
}

void Backend::endcurv(void)
{
   curveEvaluator.endmap1f();
}
