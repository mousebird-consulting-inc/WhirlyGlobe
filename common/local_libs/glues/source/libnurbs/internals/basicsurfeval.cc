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

#include "mystdio.h"
#include "types.h"
#include "basicsurfeval.h"

#ifdef __WATCOMC__
#pragma warning 726 10
#endif

void BasicSurfaceEvaluator::domain2f(REAL, REAL, REAL, REAL)
{
}

void BasicSurfaceEvaluator::polymode(long)
{
}

long BasicSurfaceEvaluator::get_output_style()
{
   return N_MESHFILL;
}

void BasicSurfaceEvaluator::range2f(long type, REAL* from, REAL* to)
{
}

void BasicSurfaceEvaluator::enable(long)
{
}

void BasicSurfaceEvaluator::disable(long)
{
}

void BasicSurfaceEvaluator::bgnmap2f(long)
{
}

void BasicSurfaceEvaluator::endmap2f(void)
{
}

void BasicSurfaceEvaluator::map2f(long, REAL, REAL, long, long,
                                  REAL, REAL, long, long, REAL*)
{
}

void BasicSurfaceEvaluator::mapgrid2f(long, REAL, REAL, long, REAL, REAL)
{
}

void BasicSurfaceEvaluator::mapmesh2f(long, long, long, long, long)
{
}

void BasicSurfaceEvaluator::evalcoord2f(long, REAL, REAL, REAL*, REAL*)
{
}

void BasicSurfaceEvaluator::evalpoint2i(long, long)
{
}

void BasicSurfaceEvaluator::bgnline(void)
{
}

void BasicSurfaceEvaluator::endline(void)
{
}

void BasicSurfaceEvaluator::bgnclosedline(void)
{
}

void BasicSurfaceEvaluator::endclosedline(void)
{
}

void BasicSurfaceEvaluator::bgntfan(void)
{
}

void BasicSurfaceEvaluator::endtfan(void)
{
}

void BasicSurfaceEvaluator::bgntmesh(void)
{
}

void BasicSurfaceEvaluator::swaptmesh(void)
{
}

void BasicSurfaceEvaluator::endtmesh(void)
{
}

void BasicSurfaceEvaluator::bgnqstrip(void)
{
}

void BasicSurfaceEvaluator::endqstrip(void)
{
}
