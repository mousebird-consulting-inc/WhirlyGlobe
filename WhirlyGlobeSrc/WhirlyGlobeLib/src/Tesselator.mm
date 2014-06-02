//
//  Tesselator.mm
//  WhirlyGlobeApp
//
//  Created by Stephen Gifford on 7/17/11.
//  Copyright 2011-2013 mousebird consulting. All rights reserved.
//

#import <list>
#import "Tesselator.h"
#import "cpp/clipper.hpp"
#import "glues.h"

using namespace Eigen;

namespace WhirlyKit
{
    
static float PolyScale = 1e14;
    
typedef struct
{
    std::vector<Point3f> pts;
    std::vector<int> vertIDs;
} TriangulationInfo;
    
// Called for every vertex
static void vertexCallback(int which,TriangulationInfo *triInfo)
{
    triInfo->vertIDs.push_back(which);
}
    
// We have this here to force pure triangles out
static void edgeFlagCallback(GLboolean flag,TriangulationInfo *triInfo)
{
}
    
// We need to add a new vertex
static void combineCallback(const GLfloat newVertex[3], const GLfloat *neighborVertex[4],
                                const GLfloat neighborWeight[4], GLfloat **outData, TriangulationInfo *triInfo)
{
    triInfo->pts.push_back(Point3f(newVertex[0],newVertex[1],0.0));
    *outData = (GLfloat *)triInfo->pts.size()-1;
}

// This is here to verify we're getting just triangles out (not strips or fans)
static void beginCallback(GLenum type,TriangulationInfo *triInfo)
{
    
}
    
static void errorCallback(GLenum error,TriangulationInfo *triInfo)
{
//    NSLog(@"Error: %d",error);
}

// This version of the tesselator runs the data through Clipper to clean it up
// Note: Doesn't handle holes
void TesselateRingClipper(const VectorRing &ring,std::vector<VectorRing> &rets)
{
    if (ring.size() < 3)
        return;
    
    Mbr testMbr;
    testMbr.addPoints(ring);
    Point2f offset = testMbr.ll();
    Point2f span(testMbr.ur().x()-testMbr.ll().x(),testMbr.ur().y()-testMbr.ll().y());
    if (span.x() == 0.0 || span.y() == 0.0)
        return;
    float scale = 1000.0/std::max(span.x(),span.y());
    testMbr.ll() -= 0.1 * span;
    testMbr.ur() += 0.1 * span;

    // Run the points through Clipper to clean things up
    ClipperLib::Polygon poly(ring.size());
    for (unsigned int ii=0;ii<ring.size();ii++)
    {
        const Point2f &pt = ring[ii];
        poly[ii] = ClipperLib::IntPoint((pt.x()-offset.x())*scale*PolyScale,(pt.y()-offset.y())*scale*PolyScale);
    }
    
    // We need an outline to make this work
    std::vector<Point2f> boundPts;
    testMbr.asPoints(boundPts);
    ClipperLib::Polygon outline(4);
    for (unsigned int ii=0;ii<boundPts.size();ii++)
    {
        const Point2f &pt = boundPts[ii];
        outline[ii] = ClipperLib::IntPoint((pt.x()-offset.x())*scale*PolyScale,(pt.y()-offset.y())*scale*PolyScale);
    }
    
    ClipperLib::Clipper clipper;
    ClipperLib::Polygons outPolys;
    clipper.AddPolygon(outline, ClipperLib::ptSubject);
    clipper.AddPolygon(poly, ClipperLib::ptClip);
    if (clipper.Execute(ClipperLib::ctDifference, outPolys, ClipperLib::pftNonZero, ClipperLib::pftNonZero))
    {
        for (unsigned int ii=1;ii<outPolys.size();ii++)
        {
            // Construct a polyline from the clipper output
            ClipperLib::Polygon &outPoly = outPolys[ii];
            
            if (outPoly.size() < 3)
                continue;
            
            GLUtesselator *tess = gluNewTess();
            
            TriangulationInfo tessInfo;
            tessInfo.pts.resize(outPoly.size());
            gluTessCallback(tess, GLU_TESS_VERTEX_DATA, (GLvoid (*) ()) &vertexCallback);
            gluTessCallback(tess, GLU_TESS_EDGE_FLAG_DATA, (GLvoid (*) ()) &edgeFlagCallback);
            gluTessCallback(tess, GLU_TESS_COMBINE_DATA, (GLvoid (*) ()) &combineCallback);
//            gluTessCallback(tess, GLU_TESS_ERROR_DATA, (GLvoid (*) ()) &errorCallback);
//            gluTessCallback(tess, GLU_TESS_BEGIN_DATA, (GLvoid (*) ()) &beginCallback);

            gluTessBeginPolygon(tess,&tessInfo);
            gluTessBeginContour(tess);

            for (unsigned int jj=0;jj<outPoly.size();jj++)
            {
                float coords[3];
                ClipperLib::IntPoint &outPt = outPoly[jj%outPoly.size()];
//                coords[0] = outPt.X/(double)(PolyScale*scale) + offset.x();
//                coords[1] = outPt.Y/(double)(PolyScale*scale) + offset.y();
                coords[0] = outPt.X;
                coords[1] = outPt.Y;
                coords[2] = 0.0;
                gluTessVertex(tess,coords,(void *)jj);
                tessInfo.pts[jj] = Point3f(coords[0],coords[1],0.0);
            }
            
            gluTessEndContour(tess);
            gluTessEndPolygon(tess);
            
            gluDeleteTess(tess);

            // Convert to triangles
            for (unsigned int ii=0;ii<tessInfo.vertIDs.size()/3;ii++)
            {
                VectorRing tri(3);
                for (unsigned int jj=0;jj<3;jj++)
                {
                    Point3f &pt = tessInfo.pts[tessInfo.vertIDs[ii*3+jj]];
                    tri[jj] = Point2f(pt.x()/(double)(PolyScale*scale) + offset.x(),pt.y()/(double)(PolyScale*scale) + offset.y());
                }
                
                // Make sure this is pointed up
                Point3f pts[3];
                for (unsigned int jj=0;jj<3;jj++)
                    pts[jj] = Point3f(tri[jj].x(),tri[jj].y(),0.0);
                Vector3f norm = (pts[1]-pts[0]).cross(pts[2]-pts[0]);
                if (norm.z() >= 0.0)
                    std::reverse(tri.begin(),tri.end());
                
                rets.push_back(tri);
            }            
        }
    }
}
    
static const float PolyScale2 = 1e6;

// New tesselator uses poly2tri
void TesselateRing(const VectorRing &ring,VectorTrianglesRef tris)
{
    if (ring.size() < 3)
        return;
    
    GLUtesselator *tess = gluNewTess();
    
    TriangulationInfo tessInfo;
    tessInfo.pts.reserve(ring.size());
    gluTessCallback(tess, GLU_TESS_VERTEX_DATA, (GLvoid (*) ()) &vertexCallback);
    gluTessCallback(tess, GLU_TESS_EDGE_FLAG_DATA, (GLvoid (*) ()) &edgeFlagCallback);
    gluTessCallback(tess, GLU_TESS_COMBINE_DATA, (GLvoid (*) ()) &combineCallback);
    //            gluTessCallback(tess, GLU_TESS_ERROR_DATA, (GLvoid (*) ()) &errorCallback);
    //            gluTessCallback(tess, GLU_TESS_BEGIN_DATA, (GLvoid (*) ()) &beginCallback);
    
    gluTessBeginPolygon(tess,&tessInfo);
    gluTessBeginContour(tess);
    
    Point2f org = ring[0];
    for (unsigned int ii=0;ii<ring.size();ii++)
    {
        const Point2f &pt = ring[ii];
        if (ii==ring.size()-1 && pt.x() == ring[0].x() && pt.y() == ring[0].y())
            continue;
        float coords[3];
        coords[0] = (pt.x()-org.x())*PolyScale2;
        coords[1] = (pt.y()-org.y())*PolyScale2;
        coords[2] = 0.0;
        gluTessVertex(tess,coords,(void *)ii);
        tessInfo.pts.push_back(Point3f(coords[0],coords[1],0.0));
//        printf("%d: (%f,%f)\n",ii, coords[0],coords[1]);
    }
    
    gluTessEndContour(tess);
    gluTessEndPolygon(tess);
    
    gluDeleteTess(tess);
    
    // Convert to triangles
//    printf("  ");
    int startPoint = tris->pts.size();
    for (unsigned int ii=0;ii<tessInfo.vertIDs.size();ii++)
    {
        Point3f &pt = tessInfo.pts[tessInfo.vertIDs[ii]];
        tris->pts.push_back(Point3f(pt.x()/PolyScale2+org.x(),pt.y()/PolyScale2+org.y(),0.0));
    }
    
    for (unsigned int ii=0;ii<tessInfo.vertIDs.size()/3;ii++)
    {
        VectorTriangles::Triangle tri;
        for (unsigned int jj=0;jj<3;jj++)
        {
            tri.pts[jj] = ii*3+jj+startPoint;
//            printf("(%f %f) ",pt.x(),pt.y());
        }
//        printf("\n");
        
        // Make sure this is pointed up
        Point3f pts[3];
        for (unsigned int jj=0;jj<3;jj++)
            pts[jj] = tris->pts[tri.pts[jj]];
        Vector3f norm = (pts[1]-pts[0]).cross(pts[2]-pts[0]);
        if (norm.z() >= 0.0)
        {
            int tmp = tri.pts[0];
            tri.pts[0] = tri.pts[2];
            tri.pts[2] = tmp;
        }

        tris->tris.push_back(tri);
    }
}
    
void TesselateLoops(const std::vector<VectorRing> &loops,VectorTrianglesRef tris)
{
    if (loops.size() < 1)
        return;
    if (loops[0].size() < 1)
        return;
    
    if (loops.size() == 1)
    {
        TesselateRing(loops[0], tris);
        return;
    }

    // Figure out the bounding box and make up an origin
    Mbr testMbr;
    for (unsigned int ii=0;ii<loops.size();ii++)
        testMbr.addPoints(loops[ii]);
    Point2f offset = testMbr.ll();
    Point2f span(testMbr.ur().x()-testMbr.ll().x(),testMbr.ur().y()-testMbr.ll().y());
    if (span.x() == 0.0 || span.y() == 0.0)
        return;
    float scale = 1000.0/std::max(span.x(),span.y());
    testMbr.ll() -= 0.1 * span;
    testMbr.ur() += 0.1 * span;
    
    ClipperLib::Clipper clipper;
    ClipperLib::Polygons outPolys;

    // Run the points through Clipper to clean things up
    std::vector<ClipperLib::Polygon> polys;
    std::vector<ClipperLib::Polygon> clipPolys;
    for (unsigned int li=0;li<loops.size();li++)
    {
        const VectorRing &ring = loops[li];
        if (ring.size() == 0)
            continue;

        ClipperLib::Polygon poly(ring.size());
        for (unsigned int ii=0;ii<ring.size();ii++)
        {
            const Point2f &pt = ring[ii];
            poly[ii] = ClipperLib::IntPoint((pt.x()-offset.x())*scale*PolyScale,(pt.y()-offset.y())*scale*PolyScale);
        }
        
        if (li > 0)
        {
            if (ring.size() > 0)
            {
                // See if it's a hole or just another loop
                // Note: This apparently isn't a good enough test to see if we've got a hole
                if (PointInPolygon(ring[0], loops[0]))
                {
                    clipPolys.push_back(poly);
                } else {
                    TesselateRing(ring, tris);
                }
            }
        } else {
            polys.push_back(poly);
        }
    }
    clipper.AddPolygons(polys, ClipperLib::ptSubject);
    clipper.AddPolygons(clipPolys, ClipperLib::ptClip);
    
    if (clipper.Execute(ClipperLib::ctDifference, outPolys, ClipperLib::pftEvenOdd, ClipperLib::pftEvenOdd))
    {
        GLUtesselator *tess = gluNewTess();
        gluTessProperty(tess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
        
        TriangulationInfo tessInfo;
        gluTessCallback(tess, GLU_TESS_VERTEX_DATA, (GLvoid (*) ()) &vertexCallback);
        gluTessCallback(tess, GLU_TESS_EDGE_FLAG_DATA, (GLvoid (*) ()) &edgeFlagCallback);
        gluTessCallback(tess, GLU_TESS_COMBINE_DATA, (GLvoid (*) ()) &combineCallback);
        //            gluTessCallback(tess, GLU_TESS_ERROR_DATA, (GLvoid (*) ()) &errorCallback);
        //            gluTessCallback(tess, GLU_TESS_BEGIN_DATA, (GLvoid (*) ()) &beginCallback);
        
        gluTessBeginPolygon(tess,&tessInfo);

        int whichVertex = 0;
        for (unsigned int ii=0;ii<outPolys.size();ii++)
        {
            // Construct a polyline from the clipper output
            ClipperLib::Polygon &outPoly = outPolys[ii];
            
            if (outPoly.size() < 3)
                continue;
            
            gluTessBeginContour(tess);

            for (unsigned int jj=0;jj<outPoly.size();jj++,whichVertex++)
            {
                float coords[3];
                ClipperLib::IntPoint &outPt = outPoly[jj%outPoly.size()];
                
                //                coords[0] = outPt.X/(double)(PolyScale*scale) + offset.x();
                //                coords[1] = outPt.Y/(double)(PolyScale*scale) + offset.y();
                coords[0] = outPt.X/(double)(PolyScale*scale) + offset.x();
                coords[1] = outPt.Y/(double)(PolyScale*scale) + offset.y();
                coords[2] = 0.0;
                
                if (jj == outPoly.size()-1 && coords[0] == tessInfo.pts[0].x() && coords[1] == tessInfo.pts[0].y())
                    continue;

                gluTessVertex(tess,coords,(void *)whichVertex);
                tessInfo.pts.push_back(Point3f(coords[0],coords[1],0.0));
            }
            
            gluTessEndContour(tess);
        }
        
        gluTessEndPolygon(tess);
        gluDeleteTess(tess);
        
        int startPoint = tris->pts.size();
        for (unsigned int ii=0;ii<tessInfo.vertIDs.size();ii++)
        {
            Point3f &pt = tessInfo.pts[tessInfo.vertIDs[ii]];
            tris->pts.push_back(Point3f(pt.x(),pt.y(),0.0));
        }
        
        for (unsigned int ii=0;ii<tessInfo.vertIDs.size()/3;ii++)
        {
            VectorTriangles::Triangle tri;
            for (unsigned int jj=0;jj<3;jj++)
                tri.pts[jj] = ii*3+jj+startPoint;
            
            // Make sure this is pointed up
            Point3f pts[3];
            for (unsigned int jj=0;jj<3;jj++)
                pts[jj] = tris->pts[tri.pts[jj]];
            Vector3f norm = (pts[1]-pts[0]).cross(pts[2]-pts[0]);
            if (norm.z() >= 0.0)
            {
                int tmp = tri.pts[0];
                tri.pts[0] = tri.pts[2];
                tri.pts[2] = tmp;
            }
            
            tris->tris.push_back(tri);
        }
    }
}
    
void TesselateRingOld(const VectorRing &ring,std::vector<VectorRing> &rets)
{
    int startRet = rets.size();
        
	// Simple cases
	if (ring.size() < 3)
		return;
    
	if (ring.size() == 3)
	{
		rets.push_back(ring);
		return;
	}
    
	// Convert to a linked list
	std::list<Point2f> poly;
	for (unsigned int ii=0;ii<ring.size();ii++)
		poly.push_back(ring[ii]);
    std::reverse(poly.begin(),poly.end());
    
	// Whittle down the polygon until there's 3 left
	while (poly.size() > 3)
	{
		std::list<Point2f>::iterator bestPt = poly.end();
		std::list<Point2f>::iterator prevBestPt = poly.end();
		std::list<Point2f>::iterator nextBestPt = poly.end();
        
		// Look for the best point
		std::list<Point2f>::iterator prevPt = poly.end(); --prevPt;
		std::list<Point2f>::iterator pt = poly.begin();
		std::list<Point2f>::iterator nextPt = poly.begin(); nextPt++;
		while (pt != poly.end())
		{
			bool valid = true;
			// First, see if this is a valid triangle
			// Pt should be on the left of prev->next
			Point2f dir0(pt->x()-prevPt->x(),pt->y()-prevPt->y());
			Point2f dir1(nextPt->x()-prevPt->x(),nextPt->y()-prevPt->y());
			float z = dir0.x()*dir1.y() - dir0.y()*dir1.x();
            
			if (z < 0.0)
				valid = false;
                        
			// Check that none of the other points fall within the proposed triangle
			if (valid)
			{
				VectorRing newTri;
				newTri.push_back(*prevPt);
				newTri.push_back(*pt);
				newTri.push_back(*nextPt);
				for (std::list<Point2f>::iterator it = poly.begin();it!=poly.end();++it)
				{
					// Obviously the three points we're going to use don't count
					if (it == prevPt || it == nextPt || it == pt)
						continue;
                    
					if (PointInPolygon(*it,newTri))
					{
						valid = false;
						break;
					} else {
                    }
				}
			}

            // any valid point will do, we're not going to optimize further
			if (valid)
			{
                bestPt = pt;
                prevBestPt = prevPt;
                nextBestPt = nextPt;
                
                break;
			}
            
			if ((++prevPt) == poly.end())
				prevPt = poly.begin();
			++pt;
			if ((++nextPt) == poly.end())
				nextPt = poly.begin();
		}
        
		// Form the triangle (bestPt-1,bestPt,bestPt+1)
		if (bestPt == poly.end())
		{
//            printf("Tesselate failure for %d input\n",(int)ring.size());
            break;
            
		} else {
			VectorRing newTri;
			newTri.push_back(*prevBestPt);
			newTri.push_back(*bestPt);
			newTri.push_back(*nextBestPt);
			rets.push_back(newTri);
		}
		poly.erase(bestPt);
	}
    
	// What's left should be a single triangle
	VectorRing lastTri;
	for (std::list<Point2f>::iterator it = poly.begin();it != poly.end();++it)
		lastTri.push_back(*it);
	rets.push_back(lastTri);
    
    for (unsigned int ii=startRet;ii<rets.size();ii++)
    {
        VectorRing &retTri = rets[ii];
        std::reverse(retTri.begin(),retTri.end());
    }
}

}
