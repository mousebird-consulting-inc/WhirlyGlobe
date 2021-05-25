/*  Tesselator.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/17/11.
 *  Copyright 2011-2021 mousebird consulting
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
 */

#import <list>
#include "glues.h"
#import "Tesselator.h"

using namespace Eigen;

namespace WhirlyKit
{
    
//static float PolyScale = 1e14;
    
typedef struct
{
    Point3fVector pts;
    std::vector<std::vector<int>> tris;
//    std::vector<int> vertIDs;
    bool newVert;
} TriangulationInfo;

static void* stdAlloc(void* userData, unsigned int size)
{
  TESS_NOTUSED(userData);
  return malloc(size);
}

static void stdFree(void* userData, void* ptr)
{
  TESS_NOTUSED(userData);
  free(ptr);
}


static const float PolyScale2 = 1e6;
    
void TesselateRing(const WhirlyKit::VectorRing &ring,VectorTrianglesRef tris)
{
    std::vector<VectorRing> rings(1);
    rings[0] = ring;
    TesselateLoops(rings, tris);
}
    
void TesselateLoops(const std::vector<VectorRing> &loops,VectorTrianglesRef tris)
{
    if (loops.size() < 1)
        return;
    if (loops[0].size() < 1)
        return;
    
    static const int vertexSize = 2;
    static const int stride = sizeof(TESSreal) * vertexSize;
    static const int verticesPerTriangle = 3;
  
    TESSalloc ma;

    ma.memalloc = stdAlloc;
    ma.memfree = stdFree;
    ma.extraVertices = 256; // realloc not provided, allow 256 extra vertic

    TESStesselator *tess = tessNewTess(&ma);
    
    Point2f org = (loops[0])[0];
    for (unsigned int li=0;li<loops.size();li++)
    {
        
        const VectorRing &ring = loops[li];
        std::vector<TESSreal> tessRing;
        for (unsigned int ii=0;ii<ring.size();ii++)
        {
            const Point2f &pt = ring[ii];
            if (ii==ring.size()-1 && pt.x() == ring[0].x() && pt.y() == ring[0].y())
                continue;
            if (ii > 0)
            {
                // We're seeing a lot of duplicates
                const Point2f &prevPt = ring[ii-1];
                if (pt.x() == prevPt.x() && pt.y() == prevPt.y())
                    continue;
            }
            tessRing.push_back(static_cast<TESSreal>((pt.x()-org.x())*PolyScale2));
            tessRing.push_back(static_cast<TESSreal>((pt.y()-org.y())*PolyScale2));
        }
        tessAddContour(tess, vertexSize, tessRing.data(), stride, (int)tessRing.size() / vertexSize);
        
    }
    tessTesselate(tess, TESS_WINDING_ODD, TESS_POLYGONS, verticesPerTriangle, vertexSize, 0);
 
    const float* verts = tessGetVertices(tess);
    const int* elems = tessGetElements(tess);
    const int nelems = tessGetElementCount(tess);

    for (int i = 0; i < nelems; i++)
    {
        const TESSindex* poly = &elems[i * verticesPerTriangle];
        VectorTriangles::Triangle triOut;
        int startPoint = (int)(tris->pts.size());
        for (int j = 0; j < verticesPerTriangle; j++) {
          if (poly[j] == TESS_UNDEF) {
            break;
          }
          const TESSreal* pos = &verts[poly[j] * vertexSize];
          tris->pts.push_back(Point3f(pos[0]/PolyScale2+org.x(), pos[1]/PolyScale2+org.y(), 0.0));
          triOut.pts[j] = j + startPoint;
        }
        
         //Make sure this is pointed up
//                   Point3f pts[3];
//                   for (unsigned int jj=0;jj<3;jj++)
//                       pts[jj] = tris->pts[triOut.pts[jj]];
//                   Vector3f norm = (pts[1]-pts[0]).cross(pts[2]-pts[0]);
//                   if (norm.z() >= 0.0)
//                   {
//                       int tmp = triOut.pts[0];
//                       triOut.pts[0] = triOut.pts[2];
//                       triOut.pts[2] = tmp;
//                   }
       
        tris->tris.push_back(triOut);
    }
 
    tessDeleteTess(tess);
    
    // Convert to triangles
    //    printf("  ");
//    int startPoint = (int)(tris->pts.size());
//    for (unsigned int ii=0;ii<tessInfo.pts.size();ii++)
//    {
//        Point3f &pt = tessInfo.pts[ii];
//        tris->pts.push_back(Point3f(pt.x()/PolyScale2+org.x(),pt.y()/PolyScale2+org.y(),0.0));
//    }
//
//    // Convert to triangles
//    for (unsigned int ii=0;ii<tessInfo.tris.size();ii++)
//    {
//        std::vector<int> &tri = tessInfo.tris[ii];
//        if (tri.size() == 3)
//        {
//            VectorTriangles::Triangle triOut;
//            for (unsigned int jj=0;jj<3;jj++)
//            {
//                triOut.pts[jj] = tri[jj]+startPoint;
//            }
//
//            // Make sure this is pointed up
//            Point3f pts[3];
//            for (unsigned int jj=0;jj<3;jj++)
//                pts[jj] = tris->pts[triOut.pts[jj]];
//            Vector3f norm = (pts[1]-pts[0]).cross(pts[2]-pts[0]);
//            if (norm.z() >= 0.0)
//            {
//                int tmp = triOut.pts[0];
//                triOut.pts[0] = triOut.pts[2];
//                triOut.pts[2] = tmp;
//            }
//
//            tris->tris.push_back(triOut);
//        }
//    }
}

}
