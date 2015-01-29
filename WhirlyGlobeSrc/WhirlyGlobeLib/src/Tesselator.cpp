//
//  Tesselator.mm
//  WhirlyGlobeApp
//
//  Created by Stephen Gifford on 7/17/11.
//  Copyright 2011-2013 mousebird consulting. All rights reserved.
//

#import <list>
#import "Tesselator.h"
#import "glues.h"

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
    
// Called for every vertex
static void vertexCallback(int which,TriangulationInfo *triInfo)
{
//    triInfo->vertIDs.push_back(which);
    std::vector<int> *tri = &(triInfo->tris.back());
    if (tri->size() == 3)
    {
        triInfo->tris.resize(triInfo->tris.size()+1);
        tri = &(triInfo->tris.back());
    }
    tri->push_back(which);
}
    
// We need to add a new vertex
static void combineCallback(const GLfloat newVertex[3], const int neighborVertex[4],
                                const GLfloat neighborWeight[4], GLfloat **outData, TriangulationInfo *triInfo)
{
//    int neighbors[4];
//    for (unsigned int ii=0;ii<4;ii++)
//        neighbors[ii] = neighborVertex[ii];
//    Point3f p0 = triInfo->pts[neighbors[0]];
//    Point3f p1 = triInfo->pts[neighbors[1]];
//    NSLog(@"NewVertex = (%f,%f), neighorVertex = (%d,%d,%d,%d), weights = (%f,%f,%f,%f)\n\tp0 = (%f,%f), p1 = (%f,%f)",newVertex[0],newVertex[1],neighbors[0],neighbors[1],neighbors[2],neighbors[3],neighborWeight[0],neighborWeight[1],neighborWeight[2],neighborWeight[3],p0.x(),p0.y(),p1.x(),p1.y());
    
    triInfo->pts.push_back(Point3f(newVertex[0],newVertex[1],0.0));
    *outData = (GLfloat *)(triInfo->pts.size()-1);
    
    triInfo->newVert = true;
}

static void beginCallback(GLenum type,TriangulationInfo *triInfo)
{
    triInfo->tris.resize(triInfo->tris.size()+1);
}
    
static void endCallback(TriangulationInfo *triInfo)
{
}
 
// This forces the tesselator to product only triangles
static void edgeFlagCallback( GLboolean flag, void *polygon_data )
{
}
    
static void errorCallback(GLenum error,TriangulationInfo *triInfo)
{
//    NSLog(@"Error: %d",error);
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
    
    GLUtesselator *tess = gluNewTess();
    
    TriangulationInfo tessInfo;
    int totPoints = 0;
    for (unsigned int ii=0;ii<loops.size();ii++)
        totPoints += loops[ii].size();
    tessInfo.pts.reserve(totPoints);
    tessInfo.newVert = false;
    gluTessCallback(tess, GLU_TESS_VERTEX_DATA, (GLvoid (*) ()) &vertexCallback);
    gluTessCallback(tess, GLU_TESS_EDGE_FLAG_DATA, (GLvoid (*) ()) &edgeFlagCallback);
    gluTessCallback(tess, GLU_TESS_COMBINE_DATA, (GLvoid (*) ()) &combineCallback);
    gluTessCallback(tess, GLU_TESS_ERROR_DATA, (GLvoid (*) ()) &errorCallback);
    gluTessCallback(tess, GLU_TESS_BEGIN_DATA, (GLvoid (*) ()) &beginCallback);
    gluTessCallback(tess, GLU_TESS_END_DATA, (GLvoid (*) ()) &endCallback);
    
    gluTessBeginPolygon(tess,&tessInfo);
    
    Point2f org = (loops[0])[0];
    for (unsigned int li=0;li<loops.size();li++)
    {
        gluTessBeginContour(tess);
        const VectorRing &ring = loops[li];
        
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
            float coords[3];
            coords[0] = (pt.x()-org.x())*PolyScale2;
            coords[1] = (pt.y()-org.y())*PolyScale2;
            coords[2] = 0.0;
            gluTessVertex(tess,coords,(void *)tessInfo.pts.size());
            tessInfo.pts.push_back(Point3f(coords[0],coords[1],0.0));
            //        printf("%d: (%f,%f)\n",ii, coords[0],coords[1]);
        }
        
        gluTessEndContour(tess);
    }
    gluTessEndPolygon(tess);
    
    gluDeleteTess(tess);
    
    // Convert to triangles
    //    printf("  ");
    int startPoint = (int)(tris->pts.size());
    for (unsigned int ii=0;ii<tessInfo.pts.size();ii++)
    {
        Point3f &pt = tessInfo.pts[ii];
        tris->pts.push_back(Point3f(pt.x()/PolyScale2+org.x(),pt.y()/PolyScale2+org.y(),0.0));
    }
    
    // Convert to triangles
    for (unsigned int ii=0;ii<tessInfo.tris.size();ii++)
    {
        std::vector<int> &tri = tessInfo.tris[ii];
        if (tri.size() == 3)
        {
            VectorTriangles::Triangle triOut;
            for (unsigned int jj=0;jj<3;jj++)
            {
                triOut.pts[jj] = tri[jj]+startPoint;
            }
            
            // Make sure this is pointed up
            Point3f pts[3];
            for (unsigned int jj=0;jj<3;jj++)
                pts[jj] = tris->pts[triOut.pts[jj]];
            Vector3f norm = (pts[1]-pts[0]).cross(pts[2]-pts[0]);
            if (norm.z() >= 0.0)
            {
                int tmp = triOut.pts[0];
                triOut.pts[0] = triOut.pts[2];
                triOut.pts[2] = tmp;
            }
            
            tris->tris.push_back(triOut);
        }
    }
}

}
