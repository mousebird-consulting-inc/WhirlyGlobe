/*
 *  WideVectorManager.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/29/14.
 *  Copyright 2011-2015 mousebird consulting.
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

#import "WideVectorManager.h"
#import "FlatMath.h"
#import "SharedAttributes.h"

using namespace WhirlyKit;
using namespace Eigen;

namespace WhirlyKit
{

WideVectorInfo::WideVectorInfo(const Dictionary &dict) :
BaseInfo(dict),color(255,255,255,255),width(2.0),repeatSize(32),edgeSize(1.0),coordType(WideVecCoordScreen),joinType(WideVecMiterJoin),
capType(WideVecButtCap),texID(EmptyIdentity),miterLimit(2.0)
{
    color = dict.getColor(MaplyColor,RGBAColor(255,255,255,255));
    width = dict.getDouble(MaplyVecWidth,2.0);
    std::string coordTypeStr = dict.getString(MaplyWideVecCoordType);
    if (!coordTypeStr.compare(MaplyWideVecCoordTypeReal))
        coordType = WideVecCoordReal;
    else if (!coordTypeStr.compare(MaplyWideVecCoordTypeScreen))
        coordType = WideVecCoordScreen;
    std::string jointTypeStr = dict.getString(MaplyWideVecJoinType);
    // Note: Not supporting this right now
//    _joinType = (WhirlyKit::WideVectorLineJoinType)[desc enumForKey:@"wideveclinejointype" values:@[@"miter",@"round",@"bevel"] default:WideVecMiterJoin];
//    std::String capTypeStr = dict.getString(MaplyWideVecCapType);
//    _capType = (WhirlyKit::WideVectorLineCapType)[desc enumForKey:@"wideveclinecaptype" values:@[@"butt",@"round",@"square"] default:WideVecButtCap];
    texID = dict.getInt(MaplyVecTexture,EmptyIdentity);
    repeatSize = dict.getDouble(MaplyWideVecTexRepeatLen,32);
    edgeSize = dict.getDouble(MaplyWideVecEdgeFalloff,1.0);
    miterLimit = dict.getDouble(MaplyWideVecMiterLimit,2.0);
}

// Turn this on for smaller texture lengths
//#define TEXTURE_RESET 1

class WideVectorBuilder
{
public:
    WideVectorBuilder(const WideVectorInfo *vecInfo,const Point3d &localCenter,const Point3d &dispCenter,const RGBAColor inColor,CoordSystemDisplayAdapter *coordAdapter)
    : vecInfo(vecInfo), angleCutoff(DegToRad(30.0)), texOffset(0.0), edgePointsValid(false), coordAdapter(coordAdapter), localCenter(localCenter), dispCenter(dispCenter)
    {
//        color = [vecInfo.color asRGBAColor];
        color = inColor;
    }

    // Two widened lines that intersect in a point.
    // Width/2 is the input
    class InterPoint
    {
    public:
        InterPoint() : texX(0.0),texYmin(0.0),texYmax(0.0),texOffset(0.0) { }
        // Construct with a single line
        InterPoint(const Point3d &p0,const Point3d &p1,const Point3d &n0,double inTexX,double inTexYmin,double inTexYmax,double inTexOffset)
        {
            c = 0;
            dir = p1 - p0;
            n = n0;
            org = p0;
            dest = p1;
            texX = inTexX;
            texYmin = inTexYmin;
            texYmax = inTexYmax;
            texOffset = inTexOffset;
        }
        
        // Return a version of the point flipped around its main axis
        InterPoint flipped()
        {
            InterPoint iPt = *this;
            iPt.n *= -1;
            iPt.texX = 1.0 - texX;
            
            return iPt;
        }
        
        // Pass in the half width to calculate the intersection point
        Point3d calcInterPt(double w2)
        {
            double t0 = c * w2;
            Point3d iPt = dir * t0 + n * w2 + org;
            
            return iPt;
        }
        
        double c;
        Point3d dir;
        Point3d n;
        Point3d org,dest;
        double texX;
        double texYmin,texYmax,texOffset;
    };
    
    // Intersect the wide lines, but return an equation to calculate the point
    bool intersectWideLines(const Point3d &p0,const Point3d &p1,const Point3d &p2,const Point3d &n0,const Point3d &n1,InterPoint &iPt0,InterPoint &iPt1,double texX,double texY0,double texY1,double texY2)
    {
        {
            iPt0.texX = texX;
            iPt0.dir = p0 - p1;
            iPt0.n = n0;
            iPt0.org = p1;
            iPt0.texYmin = texY1;
            iPt0.dest = p0;
            iPt0.texYmax = texY0;
            Point3d p01 = p0 - p1;
            Point3d n01 = n0 - n1;
            Point3d p21 = p2 - p1;
            
            double denom = (p21.y()*p01.x() - p01.y()*p21.x());
            if (denom == 0.0)
                return false;
            iPt0.c = (n01.y()*p21.x() - n01.x()*p21.y())/denom;
        }

        {
            iPt1.texX = texX;
            iPt1.dir = p2 - p1;
            iPt1.n = n1;
            iPt1.org = p1;
            iPt1.texYmin = texY1;
            iPt1.dest = p2;
            iPt1.texYmax = texY2;
            Point3d n10 = n1 - n0;
            Point3d p21 = p2 - p1;
            Point3d p01 = p0 - p1;
            double denom = p21.x()*p01.y()-p21.y()*p01.x();
            if (denom == 0.0)
                return false;
            iPt1.c = (n10.y()*p01.x() - n10.x()*p01.y())/denom;
        }
        
        return true;
    }

    // Add a rectangle to the wide drawable
    void addWideRect(WideVectorDrawable *drawable,InterPoint *verts,const Point3d &up)
    {
        int startPt = drawable->getNumPoints();

        for (unsigned int vi=0;vi<4;vi++)
        {
            InterPoint &vert = verts[vi];
            drawable->addPoint(Vector3dToVector3f(vert.org));
//            drawable->addNormal(up);
            drawable->add_p1(Vector3dToVector3f(vert.dest));
            drawable->add_n0(Vector3dToVector3f(vert.n));
            drawable->add_c0(vert.c);
            drawable->add_texInfo(vert.texX,vert.texYmin,vert.texYmax,vert.texOffset);
        }

        drawable->addTriangle(BasicDrawable::Triangle(startPt+0,startPt+1,startPt+3));
        drawable->addTriangle(BasicDrawable::Triangle(startPt+1,startPt+2,startPt+3));
    }
    
    // Add a triangle to the wide drawable
    void addWideTri(WideVectorDrawable *drawable,InterPoint *verts,const Point3d &up)
    {
        int startPt = drawable->getNumPoints();

        for (unsigned int vi=0;vi<3;vi++)
        {
            InterPoint &vert = verts[vi];
            drawable->addPoint(Vector3dToVector3f(vert.org));
//            drawable->addNormal(up);
            drawable->add_p1(Vector3dToVector3f(vert.dest));
            drawable->add_n0(Vector3dToVector3f(vert.n));
            drawable->add_c0(vert.c);
            drawable->add_texInfo(vert.texX,vert.texYmin,vert.texYmax,vert.texOffset);
        }
        
        drawable->addTriangle(BasicDrawable::Triangle(startPt+0,startPt+1,startPt+2));
    }
    
    // Build the polygons for a widened line segment
    void buildPolys(const Point3d *pa,const Point3d *pb,const Point3d *pc,const Point3d &up,BasicDrawable *drawable,bool buildSegment,bool buildJunction)
    {
        WideVectorDrawable *wideDrawable = dynamic_cast<WideVectorDrawable *>(drawable);
        
        double texLen = (*pb-*pa).norm();
        double texLen2 = 0.0;
        // Degenerate segment
        if (texLen == 0.0)
            return;
        
        // Next segment is degenerate
        if (pc)
        {
            if ((*pc-*pb).norm() == 0.0)
                pc = NULL;
        }

        // We need the normal (with respect to the line), and its inverse
        Point3d norm0 = (*pb-*pa).cross(up);
        norm0.normalize();
        Point3d revNorm0 = norm0 * -1.0;
        
        Point3d norm1(0,0,0),revNorm1(0,0,0);
        if (pc)
        {
            norm1 = (*pc-*pb).cross(up);
            norm1.normalize();
            revNorm1 = norm1 * -1.0;
            texLen2 = (*pc-*pa).norm();
        }
        
        Point3d paLocal = *pa-dispCenter;
        Point3d pbLocal = *pb-dispCenter;

        // Lengths we use to calculate texture coordinates
        double texBase = texOffset;
        double texNext = texOffset+texLen;
        double texNext2 = texOffset+texLen+texLen2;
        
#ifdef TEXTURE_RESET
        texBase = 0.0;
        texNext = texLen;
        texNext2 = texLen+texLen2;
#endif

        // Look for valid starting points.  If they're not there, make some simple ones
        if (!edgePointsValid)
        {
            e0 = InterPoint(paLocal,pbLocal,revNorm0,1.0,texBase,texNext,0.0);
            e1 = InterPoint(paLocal,pbLocal,norm0,0.0,texBase,texNext,0.0);
            edgePointsValid = true;
        }

        // Calculate points for the expanded linear
        InterPoint corners[4];
        
        InterPoint rPt0,lPt0,rPt1,lPt1;
        Point3d pcLocal = (pc ? *pc-dispCenter: Point3d(0,0,0));
        Point3d dirA = (paLocal-pbLocal).normalized();
        
        // Figure out which way the bend goes and calculate intersection points
        bool iPtsValid = false;
        double dot;
        double angleBetween = M_PI;
        if (pc)
        {
            // Compare the angle between the two segments.
            // We want to catch when the data folds back on itself.
            Point3d dirB = (pcLocal-pbLocal).normalized();
            dot = dirA.dot(dirB);
            if (dot > -0.99999998476 && dot < 0.99999998476)
                if (intersectWideLines(paLocal, pbLocal, pcLocal, norm0, norm1, rPt0, rPt1, 0.0, texBase, texNext, texNext2) &&
                    intersectWideLines(paLocal, pbLocal, pcLocal, revNorm0, revNorm1, lPt0, lPt1, 1.0, texBase, texNext, texNext2))
                {
                    iPtsValid = true;
                    angleBetween = acos(dot);
                }
        }
        
        // Points from the last round
        corners[0] = e0;
        corners[1] = e1;
        InterPoint next_e0,next_e1;

        // Really acute angles tend to break things
        if (angleBetween < 4.0 / 180.0 * M_PI)
        {
            iPtsValid = false;
            edgePointsValid = false;
        }

        // End points of the segments
        InterPoint endPt0(pbLocal,paLocal,norm0,0.0,texNext,texBase,0.0);
        InterPoint endPt1;
        if (pc)
            endPt1 = InterPoint(pbLocal,pcLocal,norm1,0.0,texNext,texNext2,0.0);

        // Set up the segment points
        if (iPtsValid)
        {
            // Bending right
            if (rPt0.c > 0.0)
            {
                corners[2] = rPt0;
                corners[3] = rPt0.flipped();
                next_e0 = rPt1.flipped();
                next_e1 = rPt1;
            } else {
                // Bending left
                corners[2] = lPt0.flipped();
                corners[3] = lPt0;
                next_e0 = lPt1;
                next_e1 = lPt1.flipped();
            }
        } else {
            corners[2] = endPt0;
            corners[3] = endPt0.flipped();
            next_e0 = endPt0.flipped();
            next_e1 = endPt0;
        }
        
        // Add the rectangles
        if (buildSegment)
            addWideRect(wideDrawable, corners, up);
        
        // Do the join polygons if we can
        // Note: Always doing bevel case (sort of)
        if (iPtsValid && buildJunction)
        {
            // An offset that makes the texture coordinates work
            double texAdjust = cos(angleBetween/2.0);
            
            // Three triangles make up the bend

            // Bending right
            if (rPt0.c > 0.0)
            {
                InterPoint triVerts[3];
                triVerts[0] = rPt0;
                triVerts[1] = endPt0.flipped();
                triVerts[2] = rPt0.flipped();
                addWideTri(wideDrawable,triVerts,up);
                
                triVerts[0] = rPt0;
                triVerts[0].texYmin = texNext;
                triVerts[0].texYmax = texNext;
                triVerts[1] = endPt1.flipped();
                triVerts[1].texYmin = texNext;
                triVerts[1].texYmax = texNext;
                triVerts[1].texOffset = texAdjust;
                triVerts[2] = endPt0.flipped();
                triVerts[2].texYmin = texNext;
                triVerts[2].texYmax = texNext;
                triVerts[2].texOffset = -texAdjust;
                addWideTri(wideDrawable,triVerts,up);
                
                triVerts[0] = rPt1;
                triVerts[1] = rPt1.flipped();
                triVerts[2] = endPt1.flipped();
                addWideTri(wideDrawable,triVerts,up);
            } else {
                // Bending left
                InterPoint triVerts[3];
                triVerts[0] = lPt0;
                triVerts[1] = lPt0.flipped();
                triVerts[2] = endPt0;
                addWideTri(wideDrawable,triVerts,up);

                triVerts[0] = lPt0;
                triVerts[0].texYmin = texNext;
                triVerts[0].texYmax = texNext;
                triVerts[1] = endPt0;
                triVerts[1].texYmin = texNext;
                triVerts[1].texYmax = texNext;
                triVerts[1].texOffset = -texAdjust;
                triVerts[2] = endPt1;
                triVerts[2].texYmin = texNext;
                triVerts[2].texYmax = texNext;
                triVerts[2].texOffset = texAdjust;
                addWideTri(wideDrawable,triVerts,up);

                triVerts[0] = lPt1;
                triVerts[1] = endPt1;
                triVerts[2] = lPt1.flipped();
                addWideTri(wideDrawable,triVerts,up);
            }
        }
        
        e0 = next_e0;
        e1 = next_e1;

#ifdef TEXTURE_RESET
        e0.texYmin -= texLen;
        e0.texYmax -= texLen;
        e1.texYmin -= texLen;
        e1.texYmax -= texLen;
#endif

        texOffset += texLen;
    }
    
    
    // Add a point to the widened linear we're building
    void addPoint(const Point3d &inPt,const Point3d &up,BasicDrawable *drawable,bool closed,bool buildSegment,bool buildJunction)
    {
        // Compare with the last point, if it's the same, toss it
        if (!pts.empty() && pts.back() == inPt && !closed)
            return;
        
        pts.push_back(inPt);
        if (pts.size() >= 3)
        {
            const Point3d &pa = pts[pts.size()-3];
            const Point3d &pb = pts[pts.size()-2];
            const Point3d &pc = pts[pts.size()-1];
            buildPolys(&pa,&pb,&pc,up,drawable,buildSegment,buildJunction);
        }
        lastUp = up;
    }
    
    // Flush out any outstanding points
    void flush(BasicDrawable *drawable,bool buildLastSegment, bool buildLastJunction)
    {
        if (pts.size() >= 2)
        {
            const Point3d &pa = pts[pts.size()-2];
            const Point3d &pb = pts[pts.size()-1];
            buildPolys(&pa, &pb, NULL, lastUp, drawable, buildLastSegment, buildLastJunction);
        }
    }

    const WideVectorInfo *vecInfo;
    CoordSystemDisplayAdapter *coordAdapter;
    RGBAColor color;
    Point3d localCenter,dispCenter;
    double angleCutoff;
    
    double texOffset;

    std::vector<Point3d> pts;
    Point3d lastUp;
    
    bool edgePointsValid;
    InterPoint e0,e1;
    //,centerAdj;
};

// Used to build up drawables
class WideVectorDrawableBuilder
{
public:
    WideVectorDrawableBuilder(Scene *scene,const WideVectorInfo *vecInfo)
    : scene(scene), vecInfo(vecInfo), drawable(NULL), centerValid(false), localCenter(0,0,0), dispCenter(0,0,0)
    {
        coordAdapter = scene->getCoordAdapter();
        coordSys = coordAdapter->getCoordSystem();
    }
    
    // Center to use for drawables we create
    void setCenter(const Point3d &newLocalCenter,const Point3d &newDispCenter)
    {
        centerValid = true;
        localCenter = newLocalCenter;
        dispCenter = newDispCenter;
    }
    
    // Build or return a suitable drawable (depending on the mode)
    BasicDrawable *getDrawable(int ptCount,int triCount)
    {
        int ptGuess = std::min(std::max(ptCount,0),(int)MaxDrawablePoints);
        int triGuess = std::min(std::max(triCount,0),(int)MaxDrawableTriangles);

        if (!drawable ||
            (drawable->getNumPoints()+ptGuess > MaxDrawablePoints) ||
            (drawable->getNumTris()+triGuess > MaxDrawableTriangles))
        {
            flush();
            
//            NSLog(@"Pts = %d, tris = %d",ptGuess,triGuess);
            WideVectorDrawable *wideDrawable = new WideVectorDrawable("Widen Vector",ptGuess,triGuess);
            drawable = wideDrawable;
            drawable->setProgram(vecInfo->programID);
            wideDrawable->setTexRepeat(vecInfo->repeatSize);
            wideDrawable->setEdgeSize(vecInfo->edgeSize);
            wideDrawable->setLineWidth(vecInfo->width);
            if (vecInfo->coordType == WideVecCoordReal)
                wideDrawable->setRealWorldWidth(vecInfo->width);
            
//            drawMbr.reset();
            drawable->setType(GL_TRIANGLES);
            vecInfo->setupBasicDrawable(drawable);
            drawable->setColor(vecInfo->color);
            if (vecInfo->texID != EmptyIdentity)
                drawable->setTexId(0, vecInfo->texID);
            if (centerValid)
            {
                Eigen::Affine3d trans(Eigen::Translation3d(dispCenter.x(),dispCenter.y(),dispCenter.z()));
                Matrix4d transMat = trans.matrix();
                drawable->setMatrix(&transMat);
            }
        }
        
        return drawable;
    }
    
    // Add the points for a linear
    void addLinear(const VectorRing &pts,const Point3d &up,bool closed)
    {
        RGBAColor color = vecInfo->color;
        WideVectorBuilder vecBuilder(vecInfo,localCenter,dispCenter,color,coordAdapter);
        
        // We'll add one on the beginning and two on the end
        //  if we're doing a closed loop.  This gets us
        //  valid junctions that match up.
        int startPoint = 0;
        if (closed)
        {
            if (pts.size() > 2)
            {
                if (pts.front() == pts.back())
                {
                    startPoint = -3;
                } else {
                    startPoint = -2;
                }
            }
        }
        
        // Guess at how many points and triangles we'll need
        int totalTriCount = 5*pts.size();
        int totalPtCount = totalTriCount * 3;
        if (totalTriCount < 0)  totalTriCount = 0;
        if (totalPtCount < 0)  totalPtCount = 0;
        
        // Work through the segments
        Point2f lastPt;
        bool validLastPt = false;
        for (int ii=startPoint;ii<(int)pts.size();ii++)
        {
            // Get the points in display space
            Point2f geoA = pts[(ii+pts.size())%pts.size()];
            
            if (validLastPt && geoA == lastPt)
                continue;

            Point3d dispPa = coordAdapter->localToDisplay(coordSys->geographicToLocal3d(GeoCoord(geoA.x(),geoA.y())));
            
            // Get a drawable ready
            int triCount = 2+3;
            int ptCount = triCount*3;
            BasicDrawable *thisDrawable = getDrawable(std::max(totalPtCount,ptCount),std::max(totalTriCount,triCount));
            totalTriCount -= triCount;
            totalPtCount -= ptCount;
            drawMbr.addPoint(geoA);
            
            bool doSegment = !closed || (ii > 0);
            bool doJunction = !closed || (ii >= 0);
            vecBuilder.addPoint(dispPa,up,thisDrawable,closed,doSegment,doJunction);
            
//            NSLog(@"Pt = (%f,%f), doSegment = %d, doJunction = %d",geoA.x(),geoA.y(),(int)doSegment,(int)doJunction);
            
            lastPt = geoA;
            validLastPt = true;
        }

        vecBuilder.flush(drawable,!closed,true);
    }
    
    // Debug verson of add linear
    void addLinearDebug()
    {
        Point3d up(0,0,1);
        VectorRing pts;
        pts.push_back(GeoCoord(0,1));
        pts.push_back(GeoCoord(0,0));
        pts.push_back(GeoCoord(1,0));
        
        RGBAColor color = vecInfo->color;
        WideVectorBuilder vecBuilder(vecInfo,Point3d(0,0,0),Point3d(0,0,0),color,coordAdapter);
        
        for (unsigned int ii=0;ii<pts.size();ii++)
        {
            // Get the points in display space
            Point2f geoA = pts[ii];
            
            Point3d dispPa(geoA.x(),geoA.y(),0.0);

            // Get a drawable ready
            int ptCount = 5;
            int triCount = 4;
            BasicDrawable *thisDrawable = getDrawable(ptCount,triCount);
            drawMbr.addPoint(geoA);

            vecBuilder.addPoint(dispPa,up,thisDrawable,false,true,true);
        }
        
        vecBuilder.flush(drawable,true,true);
    }

    // Flush out the drawables
    WideVectorSceneRep *flush(ChangeSet &changes)
    {
        flush();
        
        if (drawables.empty())
            return NULL;
        
        WideVectorSceneRep *sceneRep = new WideVectorSceneRep();
        sceneRep->fade = vecInfo->fade;
        for (unsigned int ii=0;ii<drawables.size();ii++)
        {
            Drawable *drawable = drawables[ii];
            sceneRep->drawIDs.insert(drawable->getId());
            changes.push_back(new AddDrawableReq(drawable));
        }
        
        drawables.clear();
        
        return sceneRep;
    }
    
protected:
    // Move an active drawable to the list
    void flush()
    {
        if (drawable)
        {
            drawable->setLocalMbr(drawMbr);
            drawables.push_back(drawable);
        }
        drawable = NULL;
    }

    bool centerValid;
    Point3d localCenter,dispCenter;
    Mbr drawMbr;
    Scene *scene;
    CoordSystemDisplayAdapter *coordAdapter;
    CoordSystem *coordSys;
    const WideVectorInfo *vecInfo;
    BasicDrawable *drawable;
    std::vector<BasicDrawable *> drawables;
};
    
WideVectorSceneRep::WideVectorSceneRep()
{
}
    
WideVectorSceneRep::WideVectorSceneRep(SimpleIdentity inId)
    : Identifiable(inId)
{
}

WideVectorSceneRep::~WideVectorSceneRep()
{
}

void WideVectorSceneRep::enableContents(bool enable,ChangeSet &changes)
{
    SimpleIDSet allIDs = drawIDs;
    allIDs.insert(instIDs.begin(),instIDs.end());
    for (SimpleIDSet::iterator it = allIDs.begin();
         it != allIDs.end(); ++it)
        changes.push_back(new OnOffChangeRequest(*it,enable));
}

void WideVectorSceneRep::clearContents(ChangeSet &changes)
{
    SimpleIDSet allIDs = drawIDs;
    allIDs.insert(instIDs.begin(),instIDs.end());
    for (SimpleIDSet::iterator it = allIDs.begin();
         it != allIDs.end(); ++it)
        changes.push_back(new RemDrawableReq(*it));
}

WideVectorManager::WideVectorManager()
{
    pthread_mutex_init(&vecLock, NULL);
}

WideVectorManager::~WideVectorManager()
{
    pthread_mutex_destroy(&vecLock);
    for (WideVectorSceneRepSet::iterator it = sceneReps.begin();
         it != sceneReps.end(); ++it)
        delete *it;
    sceneReps.clear();
}
    
SimpleIdentity WideVectorManager::addVectors(ShapeSet *shapes,const WideVectorInfo &vecInfo,ChangeSet &changes)
{
    WideVectorDrawableBuilder builder(scene,&vecInfo);
    
    // Calculate a center for this geometry
    GeoMbr geoMbr;
    for (ShapeSet::iterator it = shapes->begin(); it != shapes->end(); ++it)
    {
        GeoMbr thisMbr = (*it)->calcGeoMbr();
        geoMbr.expand(thisMbr);
    }
    // No data?
    if (!geoMbr.valid())
        return EmptyIdentity;
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    GeoCoord centerGeo = geoMbr.mid();
    Point3d localCenter = coordAdapter->getCoordSystem()->geographicToLocal3d(centerGeo);
    Point3d centerDisp = coordAdapter->localToDisplay(localCenter);
    builder.setCenter(localCenter,centerDisp);
    Point3d centerUp(0,0,1);
    if (!coordAdapter->isFlat())
    {
        centerUp = coordAdapter->normalForLocal(localCenter);
    }

    for (ShapeSet::iterator it = shapes->begin(); it != shapes->end(); ++it)
    {
        VectorLinearRef lin = std::dynamic_pointer_cast<VectorLinear>(*it);
        if (lin)
        {
            builder.addLinear(lin->pts,centerUp,false);
        } else {
            VectorArealRef ar = std::dynamic_pointer_cast<VectorAreal>(*it);
            if (ar)
            {
                for (const auto &loop : ar->loops)
                {
                    builder.addLinear(loop, centerUp, true);
                }
            }
        }
    }
//    builder.addLinearDebug();
    
    WideVectorSceneRep *sceneRep = builder.flush(changes);
    SimpleIdentity vecID = EmptyIdentity;
    if (sceneRep)
    {
        vecID = sceneRep->getId();
        pthread_mutex_lock(&vecLock);
        sceneReps.insert(sceneRep);
        pthread_mutex_unlock(&vecLock);
    }
    
    return vecID;
}

void WideVectorManager::enableVectors(SimpleIDSet &vecIDs,bool enable,ChangeSet &changes)
{
    pthread_mutex_lock(&vecLock);
    
    for (SimpleIDSet::iterator vit = vecIDs.begin();vit != vecIDs.end();++vit)
    {
        WideVectorSceneRep dummyRep(*vit);
        WideVectorSceneRepSet::iterator it = sceneReps.find(&dummyRep);
        if (it != sceneReps.end())
        {
            WideVectorSceneRep *vecRep = *it;
            SimpleIDSet allIDs = vecRep->drawIDs;
            allIDs.insert(vecRep->instIDs.begin(),vecRep->instIDs.end());
            for (SimpleIDSet::iterator idIt = allIDs.begin(); idIt != allIDs.end(); ++idIt)
                changes.push_back(new OnOffChangeRequest(*idIt,enable));
        }
    }
    
    pthread_mutex_unlock(&vecLock);
}
    
SimpleIdentity WideVectorManager::instanceVectors(SimpleIdentity vecID,const WideVectorInfo &vecInfo,ChangeSet &changes)
{
    SimpleIdentity newId = EmptyIdentity;
    
    pthread_mutex_lock(&vecLock);
    
    // Look for the representation
    WideVectorSceneRep dummyRep(vecID);
    WideVectorSceneRepSet::iterator it = sceneReps.find(&dummyRep);
    if (it != sceneReps.end())
    {
        WideVectorSceneRep *sceneRep = *it;
        WideVectorSceneRep *newSceneRep = new WideVectorSceneRep();
        for (SimpleIDSet::iterator idIt = sceneRep->drawIDs.begin();
             idIt != sceneRep->drawIDs.end(); ++idIt)
        {
            // Make up a BasicDrawableInstance
            BasicDrawableInstance *drawInst = new BasicDrawableInstance("WideVectorManager",*idIt,BasicDrawableInstance::ReuseStyle);
            
            // Changed color
            drawInst->setColor(vecInfo.color);
            
            // Changed visibility
            drawInst->setVisibleRange(vecInfo.minVis, vecInfo.maxVis);
            
            // Changed line width
            drawInst->setLineWidth(vecInfo.width);
            
            // Changed draw priority
            drawInst->setDrawPriority(vecInfo.drawPriority);
            
            // Note: Should set fade
            newSceneRep->instIDs.insert(drawInst->getId());
            changes.push_back(new AddDrawableReq(drawInst));
        }
        
        sceneReps.insert(newSceneRep);
        newId = newSceneRep->getId();
    }
    
    pthread_mutex_unlock(&vecLock);
    
    return newId;
}
    
void WideVectorManager::removeVectors(SimpleIDSet &vecIDs,ChangeSet &changes)
{
    pthread_mutex_lock(&vecLock);
    
    for (SimpleIDSet::iterator vit = vecIDs.begin();vit != vecIDs.end();++vit)
    {
        WideVectorSceneRep dummyRep(*vit);
        WideVectorSceneRepSet::iterator it = sceneReps.find(&dummyRep);
//        TimeInterval curTime = TimeGetCurrent();
        if (it != sceneReps.end())
        {
            WideVectorSceneRep *sceneRep = *it;

            // Porting: Fade turned off
//            if (sceneRep->fade > 0.0)
//            {
//                SimpleIDSet allIDs = sceneRep->drawIDs;
//                allIDs.insert(sceneRep->instIDs.begin(),sceneRep->instIDs.end());
//                for (SimpleIDSet::iterator it = allIDs.begin();
//                     it != allIDs.end(); ++it)
//                    changes.push_back(new FadeChangeRequest(*it, curTime, curTime+sceneRep->fade));
            
//                __block NSObject * __weak thisCanary = canary;
//
//                // Spawn off the deletion for later
//                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, sceneRep->fade * NSEC_PER_SEC),
//                               scene->getDispatchQueue(),
//                               ^{
//                                   if (thisCanary)
//                                   {
//                                       SimpleIDSet theIDs;
//                                       theIDs.insert(sceneRep->getId());
//                                       ChangeSet delChanges;
//                                       removeVectors(theIDs, delChanges);
//                                       scene->addChangeRequests(delChanges);
//                                   }
//                               }
//                               );
                
//                sceneRep->fade = 0.0;
//            } else {
                (*it)->clearContents(changes);
                sceneReps.erase(it);
                delete sceneRep;
//            }
        }
    }
    
    pthread_mutex_unlock(&vecLock);
}

}
