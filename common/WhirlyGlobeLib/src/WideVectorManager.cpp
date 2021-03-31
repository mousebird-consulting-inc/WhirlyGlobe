/*  WideVectorManager.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/29/14.
 *  Copyright 2011-2021 mousebird consulting.
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

#import "WideVectorManager.h"
#import "VectorManager.h"
#import "BasicDrawableInstanceBuilder.h"
#import "FlatMath.h"
#import "WhirlyKitLog.h"
#import "StringIndexer.h"
#import "SharedAttributes.h"
#import "WideVectorDrawableBuilder.h"

using namespace WhirlyKit;
using namespace Eigen;

namespace WhirlyKit
{
WideVectorInfo::WideVectorInfo() :
    implType(WideVecImplBasic), color(RGBAColor::white()), width(2.0),
    repeatSize(32.0), edgeSize(1.0), subdivEps(0.0),
    coordType(WideVecCoordScreen), joinType(WideVecMiterJoin), capType(WideVecButtCap),
    texID(EmptyIdentity), miterLimit(2.0), offset(0.0f)
{
}

WideVectorInfo::WideVectorInfo(const Dictionary &dict)
    : BaseInfo(dict)
{
    implType = WideVecImplBasic;
    std::string implTypeStr = dict.getString(MaplyWideVecImpl);
    if (!implTypeStr.compare(MaplyWideVecImplPerf))
        implType = WideVecImplPerf;
    color = dict.getColor(MaplyColor,RGBAColor::white());
    width = dict.getDouble(MaplyVecWidth,2.0);
    offset = (float)-dict.getDouble(MaplyWideVecOffset,0.0);
    std::string coordTypeStr = dict.getString(MaplyWideVecCoordType);
    subdivEps = dict.getDouble(MaplySubdivEpsilon,0.0);
    coordType = WideVecCoordScreen;
    if (!coordTypeStr.compare(MaplyWideVecCoordTypeReal))
        coordType = WideVecCoordReal;
    else if (!coordTypeStr.compare(MaplyWideVecCoordTypeScreen))
        coordType = WideVecCoordScreen;
    joinType = WideVecMiterJoin;
    std::string jointTypeStr = dict.getString(MaplyWideVecJoinType);
    capType = WideVecButtCap;
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
    WideVectorBuilder(const WideVectorInfo *vecInfo,
                      Point3d localCenter,
                      Point3d dispCenter,
                      const RGBAColor inColor,
                      std::vector<SimpleIdentity> maskIDs,
                      bool makeTurns,
                      CoordSystemDisplayAdapter *coordAdapter) :
          vecInfo(vecInfo),
          angleCutoff(DegToRad(30.0)),
          texOffset(0.0),
          edgePointsValid(false),
          coordAdapter(coordAdapter),
          localCenter(std::move(localCenter)),
          dispCenter(std::move(dispCenter)),
          makeDistinctTurn(makeTurns),
          maskIDs(std::move(maskIDs)),
          color(RGBAColor::white())
    {
        color = inColor;
    }

    // Two widened lines that intersect in a point.
    // Width/2 is the input
    class InterPoint
    {
    public:
        InterPoint() :
            c(0.0),
            texX(0.0),
            texYmin(0.0),
            texYmax(0.0),
            texOffset(0.0),
            offset(0.0,0.0),
            centerlineDir(1.0)
        { }

        // Construct with a single line
        InterPoint(const Point3d &p0,const Point3d &p1,const Point3d &n0,double inTexX,double inTexYmin,double inTexYmax,double inTexOffset)
        {
            c = 0;
            dir = p1 - p0;
            n = n0;
            org = p0;
            dest = p1;
            centerlineDir = 1.0;
            offset = Point2d(0.0,0.0);
            texX = inTexX;
            texYmin = inTexYmin;
            texYmax = inTexYmax;
            texOffset = inTexOffset;
        }
                
        // Pass in the half width to calculate the intersection point
        Point3d calcInterPt(double centerOffset,double w2)
        {
            double t0 = c * (centerOffset + w2);
            Point3d iPt = dir * t0 +
                          dir * w2 * offset.y() +
                          n * (centerOffset + w2) +
                          n * offset.x() +
                          org;
            
            return iPt;
        }
        
        InterPoint flipped() {
            InterPoint newPt = *this;
            newPt.n *= -1;
            
            return newPt;
        }

        // Same point, but offset along the centerline
        InterPoint nudgeAlongCenter(double nudge) {
            InterPoint newPt = *this;
            newPt.offset.y() += nudge;
            
            return newPt;
        }
        
        // Same point, but offset along the normal
        InterPoint nudgeAlongNormal(double nudge) {
            InterPoint newPt = *this;
            newPt.offset.x() += nudge;

            return newPt;
        }
        
        // Set the texture X coordinate, but otherwise just copy
        InterPoint withTexX(double newTexX) {
            InterPoint newPt = *this;
            newPt.texX = newTexX;
            
            return newPt;
        }
        
        // Set the tex min/max accordingly, but otherwise just copy
        InterPoint withTexY(double newMinTexY,double newMaxTexY) {
            InterPoint newPt = *this;
            newPt.texYmin = newMinTexY;
            newPt.texYmax = newMaxTexY;
            
            return newPt;
        }
        
        // Set the tex offset, but otherwise just copy
        InterPoint withTexOffset(double newTexOffset) {
            InterPoint newPt = *this;
            newPt.texOffset = newTexOffset;
            
            return newPt;
        }
        
        double c;
        Point3d dir;
        Point3d n;
        Point3d org,dest;
        Point2d offset;
        double centerlineDir;
        double texX;
        double texYmin,texYmax,texOffset;
    };
    
    // Intersect the wide lines, but return an equation to calculate the point
    static bool intersectWideLines(const Point3d &p0,const Point3d &p1,const Point3d &p2,
                                   const Point3d &n0,const Point3d &n1,
                                   InterPoint &iPt0,InterPoint &iPt1,
                                   double centerlineDir0, double centerlineDir1,
                                   double texX,double texY0,double texY1,double texY2)
    {
        {
            iPt0.texX = texX;
            iPt0.dir = p0 - p1;
            iPt0.n = n0;
            iPt0.centerlineDir = centerlineDir0;
            iPt0.org = p1;
            iPt0.texYmin = texY1;
            iPt0.dest = p0;
            iPt0.texYmax = texY0;
            Point3d p01 = p0 - p1;
            Point3d n01 = n0 - n1;
            Point3d p21 = p2 - p1;
            
            const double denom = (p21.y()*p01.x() - p01.y()*p21.x());
            if (denom == 0.0)
                return false;
            iPt0.c = (n01.y()*p21.x() - n01.x()*p21.y())/denom;
        }

        {
            iPt1.texX = texX;
            iPt1.dir = p2 - p1;
            iPt1.n = n1;
            iPt1.centerlineDir = centerlineDir1;
            iPt1.org = p1;
            iPt1.texYmin = texY1;
            iPt1.dest = p2;
            iPt1.texYmax = texY2;
            Point3d n10 = n1 - n0;
            Point3d p21 = p2 - p1;
            Point3d p01 = p0 - p1;
            const double denom = p21.x()*p01.y()-p21.y()*p01.x();
            if (denom == 0.0)
                return false;
            iPt1.c = (n10.y()*p01.x() - n10.x()*p01.y())/denom;
        }
        
        return true;
    }

    // Add a rectangle to the wide drawable
    void addWideRect(const WideVectorDrawableBuilderRef &drawable,InterPoint *verts,const Point3d &up)
    {
        const int startPt = drawable->getNumPoints();

        for (unsigned int vi=0;vi<4;vi++)
        {
            const InterPoint &vert = verts[vi];
            drawable->addPoint(Vector3dToVector3f(vert.org));
            drawable->addNormal(up);
            drawable->add_p1(Vector3dToVector3f(vert.dest));
            drawable->add_n0(Vector3dToVector3f(vert.n));
            drawable->add_offset(Vector3dToVector3f(Point3d(vert.offset.x(),vert.offset.y(),vert.centerlineDir)));
            drawable->add_c0(vert.c);
            drawable->add_texInfo(vert.texX,vert.texYmin,vert.texYmax,vert.texOffset);
            for (unsigned int ii=0;ii<maskEntries.size();ii++)
                drawable->addAttributeValue(maskEntries[ii], (int) maskIDs[ii]);
        }

        drawable->addTriangle(BasicDrawable::Triangle(startPt+0,startPt+1,startPt+3));
        drawable->addTriangle(BasicDrawable::Triangle(startPt+1,startPt+2,startPt+3));
    }
    
    // Add a triangle to the wide drawable
    void addWideTri(WideVectorDrawableBuilderRef drawable,InterPoint *verts,const Point3d &up)
    {
        const int startPt = drawable->getNumPoints();

        for (unsigned int vi=0;vi<3;vi++)
        {
            const InterPoint &vert = verts[vi];
            drawable->addPoint(Vector3dToVector3f(vert.org));
            drawable->addNormal(up);
            drawable->add_p1(Vector3dToVector3f(vert.dest));
            drawable->add_n0(Vector3dToVector3f(vert.n));
            drawable->add_offset(Vector3dToVector3f(Point3d(vert.offset.x(),vert.offset.y(),vert.centerlineDir)));
            drawable->add_c0(vert.c);
            drawable->add_texInfo(vert.texX,vert.texYmin,vert.texYmax,vert.texOffset);
            for (unsigned int ii=0;ii<maskEntries.size();ii++)
                drawable->addAttributeValue(maskEntries[ii], (int) maskIDs[ii]);
        }
        
        drawable->addTriangle(BasicDrawable::Triangle(startPt+0,startPt+1,startPt+2));
    }
    
    // Build the polygons for a widened line segment
    void buildPolys(const Point3d *pa,const Point3d *pb,const Point3d *pc,const Point3d &up,WideVectorDrawableBuilderRef wideDrawable,bool buildSegment,bool buildJunction)
    {
        double texLen = (*pb-*pa).norm();
        double texLen2 = 0.0;
        // Degenerate segment
        if (texLen == 0.0)
            return;
        
        // Next segment is degenerate
        if (pc)
        {
            if ((*pc-*pb).norm() == 0.0)
                pc = nullptr;
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
            e0 = InterPoint(paLocal,pbLocal,revNorm0,1.0,texBase,texNext,1.0);
            e1 = e0.nudgeAlongNormal(-2.0).withTexX(0.0);
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
                if (intersectWideLines(paLocal, pbLocal, pcLocal, norm0, norm1, rPt0, rPt1, -1.0, -1.0, 0.0, texBase, texNext, texNext2) &&
                    intersectWideLines(paLocal, pbLocal, pcLocal, revNorm0, revNorm1, lPt0, lPt1, 1.0, 1.0, 1.0, texBase, texNext, texNext2))
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
        InterPoint endPt0(pbLocal,paLocal,revNorm0,1.0,texNext,texBase,1.0);
        InterPoint endPt1;
        if (pc)
            endPt1 = InterPoint(pbLocal,pcLocal,norm1,1.0,texNext,texNext2,1.0);

        // Set up the segment points
        if (iPtsValid)
        {
            // Bending right
            if (rPt0.c > 0.0)
            {
                corners[3] = rPt0.nudgeAlongNormal(-2.0).withTexX(1.0);
                corners[2] = rPt0;
                next_e0 = rPt1.nudgeAlongNormal(-2.0).withTexX(1.0);
                next_e1 = rPt1;
            } else {
                // Bending left
                corners[3] = lPt0;
                corners[2] = lPt0.nudgeAlongNormal(-2.0).withTexX(0.0);
                next_e0 = lPt1;
                next_e1 = lPt1.nudgeAlongNormal(-2.0).withTexX(0.0);
            }
        } else {
            corners[3] = endPt0;
            corners[2] = endPt0.nudgeAlongNormal(-2.0).withTexX(0.0);
            next_e0 = endPt0;
            next_e1 = endPt0.nudgeAlongNormal(-2.0).withTexX(0.0);
        }
        
        // TODO: Revisit the texture adjustment around corners
        //       The problem is the corners can be much large depending on offset and width
        double texAdjust = 0.0;
        // Do the join polygons if we can
        if (iPtsValid && buildJunction)
        {
            WideVectorLineJoinType joinType = vecInfo->joinType;
            // Switch to a bevel join if the angle is too great for a miter
            double miterLimit = vecInfo->miterLimit;
            if (joinType == WideVecMiterJoin && angleBetween > (M_PI-miterLimit*M_PI/180.0))
                joinType = WideVecBevelJoin;
            // We don't do bevels below 30 degrees
            if (joinType == WideVecBevelJoin && angleBetween > 150.0 / 180.0 * M_PI)
                joinType = WideVecMiterJoin;

            // An offset that makes the texture coordinates work around corners
            texAdjust = cos(angleBetween/2.0);

            switch (joinType)
            {
                case WideVecBevelJoin:
                {
                    // Bending right
                    if (rPt0.c > 0.0)
                    {
                        InterPoint triVerts[3];

                        InterPoint r0 = corners[2];
                        InterPoint r1 = next_e1;
                        InterPoint l0 = corners[3];
                        InterPoint l1 = corners[3].nudgeAlongCenter(-1.0);
                        InterPoint l2 = next_e0.nudgeAlongCenter(-1.0);
                        InterPoint l3 = next_e0;

                        triVerts[0] = r0.withTexY(texNext,texNext);
                        triVerts[1] = l1.withTexY(texNext,texNext);
                        triVerts[2] = l0.withTexY(texNext,texNext);
                        addWideTri(wideDrawable,triVerts,up);

                        triVerts[0] = r0.withTexY(texNext,texNext);
                        triVerts[1] = l2.withTexY(texNext,texNext);
                        triVerts[2] = l1.withTexY(texNext,texNext);
                        addWideTri(wideDrawable,triVerts,up);

                        triVerts[0] = r0.withTexY(texNext,texNext);
                        triVerts[1] = r1.withTexY(texNext,texNext);
                        triVerts[2] = l2.withTexY(texNext,texNext);
                        addWideTri(wideDrawable,triVerts,up);

                        triVerts[0] = r1.withTexY(texNext,texNext);
                        triVerts[1] = l3.withTexY(texNext,texNext);
                        triVerts[2] = l2.withTexY(texNext,texNext);
                        addWideTri(wideDrawable,triVerts,up);
                    } else {
                        // Bending left
                        InterPoint triVerts[3];

                        InterPoint l0 = corners[3];
                        InterPoint l1 = next_e0;
                        InterPoint r0 = corners[2];
                        InterPoint r1 = corners[2].nudgeAlongCenter(-1.0);
                        InterPoint r2 = next_e1.nudgeAlongCenter(-1.0);
                        InterPoint r3 = next_e1;

                        triVerts[0] = l0.withTexY(texNext,texNext);
                        triVerts[1] = r0.withTexY(texNext,texNext);
                        triVerts[2] = r1.withTexY(texNext,texNext);
                        addWideTri(wideDrawable,triVerts,up);

                        triVerts[0] = l0.withTexY(texNext,texNext);
                        triVerts[1] = r1.withTexY(texNext,texNext);
                        triVerts[2] = r2.withTexY(texNext,texNext);
                        addWideTri(wideDrawable,triVerts,up);

                        triVerts[0] = l0.withTexY(texNext,texNext);
                        triVerts[1] = r2.withTexY(texNext,texNext);
                        triVerts[2] = l1.withTexY(texNext,texNext);
                        addWideTri(wideDrawable,triVerts,up);

                        triVerts[0] = l1.withTexY(texNext,texNext);
                        triVerts[1] = r2.withTexY(texNext,texNext);
                        triVerts[2] = r3.withTexY(texNext,texNext);
                        addWideTri(wideDrawable,triVerts,up);
                    }
                }
                    break;
                case WideVecMiterJoin:
                {
                    InterPoint triVerts[3];

                    // Bending right
                    if (rPt0.c > 0.0) {
                        const double texYmin = lPt0.texYmin;
                        const double textYmax = lPt0.texYmax;
                        triVerts[0] = lPt0.withTexY(texYmin,textYmax);
                        triVerts[1] = corners[3].withTexY(texYmin,textYmax);
                        triVerts[2] = corners[2].withTexY(texYmin,textYmax);
                        addWideTri(wideDrawable,triVerts,up);

                        triVerts[0] = next_e1.withTexY(texYmin,textYmax);
                        triVerts[1] = next_e0.withTexY(texYmin,textYmax);
                        triVerts[2] = lPt1.withTexY(texYmin,textYmax);
                        addWideTri(wideDrawable,triVerts,up);
                    } else {
                        // Bending left
                        const double texYmin = rPt0.texYmin;
                        const double textYmax = rPt0.texYmax;
                        triVerts[0] = corners[3].withTexY(texYmin,textYmax);
                        triVerts[1] = corners[2].withTexY(texYmin,textYmax);
                        triVerts[2] = rPt0.withTexY(texYmin,textYmax);
                        addWideTri(wideDrawable,triVerts,up);

                        triVerts[0] = rPt1.withTexY(texYmin,textYmax);
                        triVerts[1] = next_e1.withTexY(texYmin,textYmax);
                        triVerts[2] = next_e0.withTexY(texYmin,textYmax);
                        addWideTri(wideDrawable,triVerts,up);
                    }
                }
                    break;
                case WideVecRoundJoin:
                    break;
            }
        }
        
        // Add the rectangles
        if (buildSegment)
            addWideRect(wideDrawable, corners, up);
        
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
    void addPoint(const Point3d &inPt,const Point3d &up,WideVectorDrawableBuilderRef &drawable,bool closed,bool buildSegment,bool buildJunction)
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
    void flush(WideVectorDrawableBuilderRef &drawable,bool buildLastSegment, bool buildLastJunction)
    {
        if (pts.size() >= 2)
        {
            const Point3d &pa = pts[pts.size()-2];
            const Point3d &pb = pts[pts.size()-1];
            buildPolys(&pa, &pb, nullptr, lastUp, drawable, buildLastSegment, buildLastJunction);
        }
    }

    const WideVectorInfo *vecInfo;
    CoordSystemDisplayAdapter *coordAdapter;
    RGBAColor color;
    std::vector<SimpleIdentity> maskEntries;
    std::vector<SimpleIdentity> maskIDs;
    Point3d localCenter,dispCenter;
    double angleCutoff;
    bool makeDistinctTurn;
    
    double texOffset;

    Point3dVector pts;
    Point3d lastUp;
    
    bool edgePointsValid;
    InterPoint e0,e1;
    //,centerAdj;
};

// Used to build up drawables
class WideVectorDrawableConstructor
{
public:
    WideVectorDrawableConstructor(SceneRenderer *sceneRender,Scene *scene,const WideVectorInfo *vecInfo,int numMaskIDs)
    : sceneRender(sceneRender), scene(scene), vecInfo(vecInfo), drawable(nullptr), centerValid(false), localCenter(0,0,0), dispCenter(0,0,0), numMaskIDs(numMaskIDs)
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
    WideVectorDrawableBuilderRef getDrawable(int ptCount,int triCount,int ptCountAllocate,int triCountAllocate,int clineCount)
    {
        if (vecInfo->implType == WideVecImplPerf) {
            // Performance mode uses instancing and makes the renderer do the work
            if (!drawable ||
                drawable->getCenterLineCount()+clineCount > drawable->maxInstances())
            {
                flush();

                WideVectorDrawableBuilderRef wideDrawable = sceneRender->makeWideVectorDrawableBuilder("Wide Vector");
                wideDrawable->Init(ptCountAllocate,triCountAllocate,clineCount,
                                   vecInfo->implType,
                                   !scene->getCoordAdapter()->isFlat(),
                                   vecInfo);
                drawable = wideDrawable;
                wideDrawable->setTexRepeat(vecInfo->repeatSize);
                wideDrawable->setEdgeSize(vecInfo->edgeSize);
                wideDrawable->setLineWidth(vecInfo->width);
                wideDrawable->setLineOffset(vecInfo->offset);
                if (vecInfo->widthExp)
                    wideDrawable->setWidthExpression(vecInfo->widthExp);
                if (vecInfo->opacityExp)
                    wideDrawable->setOpacityExpression(vecInfo->opacityExp);
                if (vecInfo->colorExp)
                    wideDrawable->setColorExpression(vecInfo->colorExp);
                if (vecInfo->offsetExp)
                    wideDrawable->setOffsetExpression(vecInfo->offsetExp);
                maskEntries.resize(numMaskIDs);
                for (unsigned int ii=0;ii<maskEntries.size();ii++)
                    maskEntries[ii] = wideDrawable->addAttribute(BDIntType, a_maskNameIDs[ii], sceneRender->getSlotForNameID(a_maskNameIDs[ii]), ptCount);

                drawable->setColor(vecInfo->color);

                int baseTexId = 0;
                if (vecInfo->texID != EmptyIdentity)
                    drawable->setTexId(baseTexId++, vecInfo->texID);
                if (centerValid)
                {
                    Eigen::Affine3d trans(Eigen::Translation3d(dispCenter.x(),dispCenter.y(),dispCenter.z()));
                    Matrix4d transMat = trans.matrix();
                    drawable->setMatrix(&transMat);
                }
            }
        } else {
            // Basic mode builds up a lot more geometry
            int ptGuess = std::min(std::max(ptCount,0),(int)MaxDrawablePoints);
            int triGuess = std::min(std::max(triCount,0),(int)MaxDrawableTriangles);

            if (!drawable ||
                (drawable->getNumPoints()+ptGuess > MaxDrawablePoints) ||
                (drawable->getNumTris()+triGuess > MaxDrawableTriangles))
            {
                flush();
                
    //            NSLog(@"Pts = %d, tris = %d",ptGuess,triGuess);
                int ptAlloc = std::min(std::max(ptCountAllocate,0),(int)MaxDrawablePoints);
                int triAlloc = std::min(std::max(triCountAllocate,0),(int)MaxDrawableTriangles);
                WideVectorDrawableBuilderRef wideDrawable = sceneRender->makeWideVectorDrawableBuilder("Wide Vector");
                wideDrawable->Init(ptAlloc,triAlloc,0,
                                   vecInfo->implType,
                                   !scene->getCoordAdapter()->isFlat(),
                                   vecInfo);
                drawable = wideDrawable;
                wideDrawable->setTexRepeat(vecInfo->repeatSize);
                wideDrawable->setEdgeSize(vecInfo->edgeSize);
                wideDrawable->setLineWidth(vecInfo->width);
                wideDrawable->setLineOffset(vecInfo->offset);
    //            drawMbr.reset();
                if (vecInfo->widthExp)
                    wideDrawable->setWidthExpression(vecInfo->widthExp);
                if (vecInfo->opacityExp)
                    wideDrawable->setOpacityExpression(vecInfo->opacityExp);
                if (vecInfo->colorExp)
                    wideDrawable->setColorExpression(vecInfo->colorExp);
                if (vecInfo->offsetExp)
                    wideDrawable->setOffsetExpression(vecInfo->offsetExp);
                maskEntries.resize(numMaskIDs);
                for (unsigned int ii=0;ii<maskEntries.size();ii++)
                    maskEntries[ii] = wideDrawable->addAttribute(BDIntType, a_maskNameIDs[ii], sceneRender->getSlotForNameID(a_maskNameIDs[ii]), ptAlloc);

                drawable->setColor(vecInfo->color);
                int baseTexId = 0;
                if (vecInfo->texID != EmptyIdentity)
                    drawable->setTexId(baseTexId++, vecInfo->texID);
                if (centerValid)
                {
                    Eigen::Affine3d trans(Eigen::Translation3d(dispCenter.x(),dispCenter.y(),dispCenter.z()));
                    Matrix4d transMat = trans.matrix();
                    drawable->setMatrix(&transMat);
                }
            }
        }
        
        return drawable;
    }
    
    // Add the points for a linear
    void addLinear(const VectorRing &pts,
                   const Point3d &up,
                   const std::vector<SimpleIdentity> &maskIDs,
                   bool closed)
    {
        if (vecInfo->implType == WideVecImplPerf) {
            // Performance mode makes the renderer do the work
            
            // Clean up the points first
            VectorRing newPts;
            newPts.reserve(pts.size());
            for (unsigned int ii=0;ii<pts.size();ii++) {
                // Don't allow duplicate points
                if (ii > 0 && pts[ii] == pts[ii-1])
                    continue;
                
                // If it's a closed shape, no duplicates there either
                if (closed && (ii == pts.size()-1) && (pts.front() == pts.back()))
                    continue;

                newPts.push_back(pts[ii]);
            }
            if (newPts.size() < 2)
                return;

            // We're instancing, so we only need a few points and triangles
            WideVectorDrawableBuilderRef thisDrawable = getDrawable(8,6,8,6,pts.size()+1);
            drawable = thisDrawable;

            if (drawable->getNumTris() == 0) {
                // 8 points and 6 triangles.
                // Many of the points can't be shared because the end caps
                //  will be handled differently by the fragment shader
                
                // End cap: vertices [0,3], polygon 0
                drawable->addInstancePoint(Point3f(-1.0,-2.0,0.0),0,0);
                drawable->addInstancePoint(Point3f(1.0,-2.0,0.0),1,0);
                drawable->addInstancePoint(Point3f(-1.0,-1.0,0.0),2,0);
                drawable->addInstancePoint(Point3f(1.0,-1.0,0.0),3,0);
                drawable->addTriangle(BasicDrawable::Triangle(0,3,1));
                drawable->addTriangle(BasicDrawable::Triangle(0,2,3));

                // Middle segment: vertices [4,7], polygon 1
                drawable->addInstancePoint(Point3f(-1.0,-1.0,0.0),4,1);
                drawable->addInstancePoint(Point3f(1.0,-1.0,0.0),5,1);
                drawable->addInstancePoint(Point3f(-1.0,1.0,0.0),6,1);
                drawable->addInstancePoint(Point3f(1.0,1.0,0.0),7,1);
                drawable->addTriangle(BasicDrawable::Triangle(4,7,5));
                drawable->addTriangle(BasicDrawable::Triangle(4,6,7));

                // End cap: vertices [8,11], polygon 2
                drawable->addInstancePoint(Point3f(-1.0,1.0,0.0),8,2);
                drawable->addInstancePoint(Point3f(1.0,1.0,0.0),9,2);
                drawable->addInstancePoint(Point3f(-1.0,2.0,0.0),10,2);
                drawable->addInstancePoint(Point3f(1.0,2.0,0.0),11,2);
                drawable->addTriangle(BasicDrawable::Triangle(8,11,9));
                drawable->addTriangle(BasicDrawable::Triangle(8,10,11));
            }
            
            // Run through the points, adding centerline instances
            double len = 0.0;
            int startPt = drawable->getCenterLineCount();
            for (unsigned int ii=0;ii<newPts.size();ii++) {
                const auto &pt = newPts[ii];

                Point3d localPa = coordSys->geographicToLocal3d(GeoCoord(pt.x(),pt.y()));
                Point3d dispPa = coordAdapter->localToDisplay(localPa);

                int prev = startPt + ii - 1;
                if (ii == 0) {
                    prev = closed ? startPt + newPts.size() - 1 : -1;
                }
                int next = startPt + ii + 1;
                if (ii == newPts.size()-1) {
                    next = closed ? startPt : -1;
                }

                drawable->addCenterLine(dispPa,up,len,vecInfo->color,maskIDs,prev,next);
                
                if (ii<newPts.size()-1)
                    len += (newPts[ii+1] - newPts[ii]).norm();
            }
        } else {
            // We'll add one on the beginning and two on the end
            //  if we're doing a closed loop.  This gets us
            //  valid junctions that match up.
            int startPoint = 0;
            bool makeDistinctTurns = true;
            if (closed)
            {
                // Note: We need this so we don't lose one turn
                //       This could be optimized
                makeDistinctTurns = true;
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
     
            RGBAColor color = vecInfo->color;
            WideVectorBuilder vecBuilder(vecInfo,
                                         localCenter,
                                         dispCenter,
                                         color,
                                         maskIDs,
                                         makeDistinctTurns,
                                         coordAdapter);

            // Guess at how many points and triangles we'll need
            int totalTriCount = (int)(5*pts.size());
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

                Point3d localPa = coordSys->geographicToLocal3d(GeoCoord(geoA.x(),geoA.y()));
                Point3d dispPa = coordAdapter->localToDisplay(localPa);
                Point3d thisUp = up;
                if (!coordAdapter->isFlat())
                    thisUp = coordAdapter->normalForLocal(localPa);
                
                // Get a drawable ready
                int triCount = 2+3;
                int ptCount = triCount*3;
                WideVectorDrawableBuilderRef thisDrawable = getDrawable(ptCount,triCount,totalPtCount,totalTriCount,0);
                vecBuilder.maskEntries = maskEntries;
                totalTriCount -= triCount;
                totalPtCount -= ptCount;
                drawMbr.addPoint(geoA);
                
                bool doSegment = !closed || (ii > 0);
                bool doJunction = !closed || (ii >= 0);
                vecBuilder.addPoint(dispPa,thisUp,thisDrawable,closed,doSegment,doJunction);
                
    //            NSLog(@"Pt = (%f,%f), doSegment = %d, doJunction = %d",geoA.x(),geoA.y(),(int)doSegment,(int)doJunction);
                
                lastPt = geoA;
                validLastPt = true;
            }

            vecBuilder.flush(drawable,!closed,true);
        }
    }
    
    // Debug version of add linear
    void addLinearDebug()
    {
        const Point3d up(0,0,1);
        const VectorRing pts = {
                {0,1},
                {0,0},
                {1,0} };

        const RGBAColor color = vecInfo->color;
        std::vector<SimpleIdentity> maskIDs;
        WideVectorBuilder vecBuilder(vecInfo,
                                     Point3d(0,0,0),
                                     Point3d(0,0,0),
                                     color,
                                     maskIDs,
                                     false,
                                     coordAdapter);
        
        for (const auto &geoA : pts)
        {
            // Get the points in display space
            const Point3d dispPa(geoA.x(),geoA.y(),0.0);

            // Get a drawable ready
            const int ptCount = 5;
            const int triCount = 4;
            WideVectorDrawableBuilderRef thisDrawable = getDrawable(ptCount,triCount,ptCount,triCount,0);
            vecBuilder.maskEntries = maskEntries;
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
            return nullptr;
        
        const TimeInterval curTime = scene->getCurrentTime();
        
        WideVectorSceneRep *sceneRep = new WideVectorSceneRep();
        sceneRep->fade = vecInfo->fade;
        for (const auto &drawable : drawables)
        {
            if (auto drawID = drawable->getBasicDrawableID())
                sceneRep->drawIDs.insert(drawID);
            if (auto drawID = drawable->getInstanceDrawableID())
                sceneRep->drawIDs.insert(drawID);
            if (vecInfo->fade > 0.0)
                drawable->setFade(curTime,curTime+vecInfo->fade);
            if (auto draw = drawable->getBasicDrawable())
                changes.push_back(new AddDrawableReq(draw));
            if (auto draw = drawable->getInstanceDrawable())
                changes.push_back(new AddDrawableReq(draw));
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
        drawable = nullptr;
    }

    bool centerValid;
    int numMaskIDs;
    std::vector<SimpleIdentity> maskEntries;
    Point3d localCenter,dispCenter;
    Mbr drawMbr;
    SceneRenderer *sceneRender;
    Scene *scene;
    CoordSystemDisplayAdapter *coordAdapter;
    CoordSystem *coordSys;
    const WideVectorInfo *vecInfo;
    WideVectorDrawableBuilderRef drawable;
    std::vector<WideVectorDrawableBuilderRef> drawables;
};
    
WideVectorSceneRep::WideVectorSceneRep()
    : fade(0.0)
{
}
    
WideVectorSceneRep::WideVectorSceneRep(SimpleIdentity inId)
    : Identifiable(inId), fade(0.0)
{
}

WideVectorSceneRep::~WideVectorSceneRep()
{
}

void WideVectorSceneRep::enableContents(bool enable,ChangeSet &changes)
{
    SimpleIDSet allIDs = drawIDs;
    allIDs.insert(instIDs.begin(),instIDs.end());
    for (const auto &it : allIDs)
        changes.push_back(new OnOffChangeRequest(it,enable));
}

void WideVectorSceneRep::clearContents(ChangeSet &changes,TimeInterval when)
{
    SimpleIDSet allIDs = drawIDs;
    allIDs.insert(instIDs.begin(),instIDs.end());
    for (const auto &it : allIDs)
        changes.push_back(new RemDrawableReq(it,when));
}

WideVectorManager::WideVectorManager()
{
}

WideVectorManager::~WideVectorManager()
{
    std::lock_guard<std::mutex> guardLock(lock);

    for (auto it : sceneReps)
        delete it;
    sceneReps.clear();
}
    
SimpleIdentity WideVectorManager::addVectors(const std::vector<VectorShapeRef> &shapes,const WideVectorInfo &vecInfo,ChangeSet &changes)
{
    // Calculate a center for this geometry
    bool hasMaskIDs = false;
    GeoMbr geoMbr;
    for (const auto &shape : shapes)
    {
        if (shape->getAttrDict()->hasField("maskID0"))
            hasMaskIDs = true;
        geoMbr.expand(shape->calcGeoMbr());
    }
    // No data?
    if (!geoMbr.valid())
        return EmptyIdentity;

    WideVectorDrawableConstructor builder(renderer,scene,&vecInfo,hasMaskIDs ? WhirlyKitMaxMasks : 0);

    const GeoCoord centerGeo = geoMbr.mid();

    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    const Point3d localCenter = coordAdapter->getCoordSystem()->geographicToLocal3d(centerGeo);
    const Point3d centerDisp = coordAdapter->localToDisplay(localCenter);
    const auto centerUp = coordAdapter->isFlat() ? Point3d(0,0,1) : coordAdapter->normalForLocal(localCenter);
    
    builder.setCenter(localCenter,centerDisp);

    for (const auto &shape : shapes)
    {
        // Look for mask IDs.
        // Only support 2 for now
        std::vector<SimpleIdentity> maskIDs;
        if (hasMaskIDs) {
            for (unsigned int ii=0;ii<2;ii++) {
                std::string attrName = "maskID" + std::to_string(ii);
                if (shape->getAttrDict()->hasField(attrName))
                    maskIDs.push_back(shape->getAttrDict()->getInt64(attrName));
            }
        }
        // If there's not enough masks, but there is one, then fill in the rest
        if (!maskIDs.empty() && maskIDs.size() < WhirlyKitMaxMasks) {
            while (maskIDs.size() < WhirlyKitMaxMasks)
                maskIDs.push_back(maskIDs.front());
        }
        
        if (const auto lin = std::dynamic_pointer_cast<VectorLinear>(shape))
        {
            builder.addLinear(lin->pts, centerUp, maskIDs, false);
        }
        else if (const auto ar = dynamic_cast<VectorAreal*>(shape.get()))
        {
            for (const auto &loop : ar->loops)
            {
                if (loop.size() > 2 && (loop.begin() != loop.end() && vecInfo.implType != WideVecImplPerf))
                {
                    // Just tack on another point at the end.  Kind of dumb, but easy.
                    VectorRing newLoop = loop;
                    newLoop.push_back(loop[0]);
                    builder.addLinear(newLoop, centerUp, maskIDs, true);
                } else
                    builder.addLinear(loop, centerUp, maskIDs, true);
            }
        }
    }
//    builder.addLinearDebug();
    
    SimpleIdentity vecID = EmptyIdentity;
    if (auto sceneRep = builder.flush(changes))
    {
        vecID = sceneRep->getId();
        std::lock_guard<std::mutex> guardLock(lock);
        sceneReps.insert(sceneRep);
    }
    
    return vecID;
}

void WideVectorManager::enableVectors(SimpleIDSet &vecIDs,bool enable,ChangeSet &changes)
{
    std::lock_guard<std::mutex> guardLock(lock);

    for (const auto &vit : vecIDs)
    {
        WideVectorSceneRep dummyRep(vit);
        const auto it = sceneReps.find(&dummyRep);
        if (it != sceneReps.end())
        {
            const WideVectorSceneRep *vecRep = *it;
            SimpleIDSet allIDs = vecRep->drawIDs;
            allIDs.insert(vecRep->instIDs.begin(),vecRep->instIDs.end());
            for (const auto &id : allIDs)
                changes.push_back(new OnOffChangeRequest(id,enable));
        }
    }
}
    
SimpleIdentity WideVectorManager::instanceVectors(SimpleIdentity vecID,const WideVectorInfo &vecInfo,ChangeSet &changes)
{
    SimpleIdentity newId = EmptyIdentity;
    
    std::lock_guard<std::mutex> guardLock(lock);

    // Look for the representation
    WideVectorSceneRep dummyRep(vecID);
    const auto it = sceneReps.find(&dummyRep);
    if (it != sceneReps.end())
    {
        const WideVectorSceneRep *sceneRep = *it;
        auto newSceneRep = new WideVectorSceneRep();
        for (const auto &id : sceneRep->drawIDs)
        {
            // Make up a BasicDrawableInstance
            auto drawInst = renderer->makeBasicDrawableInstanceBuilder("Wide Vector Manager");
            drawInst->setMasterID(id,BasicDrawableInstance::ReuseStyle);

            // Changed enable
            drawInst->setOnOff(vecInfo.enable);
            
            // Changed color
            drawInst->setColor(vecInfo.color);
            
            // Changed visibility
            drawInst->setVisibleRange(vecInfo.minVis, vecInfo.maxVis);
            
            // Changed line width
            drawInst->setLineWidth(vecInfo.width);
            
            // Changed offset
//            drawInst->setLineOffset(vecInfo.offset);
            
            // Changed draw order
            drawInst->setDrawOrder(vecInfo.drawOrder);
            
            // Changed draw priority
            drawInst->setDrawPriority(vecInfo.drawPriority);
            
            // Note: Should set fade
            newSceneRep->instIDs.insert(drawInst->getDrawableID());
            changes.push_back(new AddDrawableReq(drawInst->getDrawable()));
        }
        
        sceneReps.insert(newSceneRep);
        newId = newSceneRep->getId();
    }
    
    return newId;
}

void WideVectorManager::changeVectors(SimpleIdentity vecID,const WideVectorInfo &vecInfo,ChangeSet &changes)
{
    std::lock_guard<std::mutex> guardLock(lock);

    WideVectorSceneRep dummyRep(vecID);
    const auto it = sceneReps.find(&dummyRep);
    if (it != sceneReps.end())
    {
        const auto sceneRep = *it;

        // Make sure we change both drawables and instances
        SimpleIDSet allIDs = sceneRep->drawIDs;
        allIDs.insert(sceneRep->instIDs.begin(),sceneRep->instIDs.end());

        for (auto id : allIDs)
        {
            // Changed color
            changes.push_back(new ColorChangeRequest(id, vecInfo.color));
            
            // Changed visibility
            if (vecInfo.minVis != DrawVisibleInvalid || vecInfo.maxVis != DrawVisibleInvalid)
            {
                changes.push_back(new VisibilityChangeRequest(id, vecInfo.minVis, vecInfo.maxVis));
            }
            
            // Changed line width
            changes.push_back(new LineWidthChangeRequest(id, vecInfo.width));
            
            // Changed draw priority
            changes.push_back(new DrawPriorityChangeRequest(id, vecInfo.drawPriority));
            
            // Changed draw order
            changes.push_back(new DrawOrderChangeRequest(id, vecInfo.drawOrder));
        }
    }
}

void WideVectorManager::removeVectors(SimpleIDSet &vecIDs,ChangeSet &changes)
{
    std::lock_guard<std::mutex> guardLock(lock);

    TimeInterval curTime = scene->getCurrentTime();
    for (SimpleIDSet::iterator vit = vecIDs.begin();vit != vecIDs.end();++vit)
    {
        WideVectorSceneRep dummyRep(*vit);
        const auto it = sceneReps.find(&dummyRep);
        if (it != sceneReps.end())
        {
            WideVectorSceneRep *sceneRep = *it;

            TimeInterval removeTime = 0.0;
            if (sceneRep->fade > 0.0)
            {
                SimpleIDSet allIDs = sceneRep->drawIDs;
                allIDs.insert(sceneRep->instIDs.begin(),sceneRep->instIDs.end());
                for (const auto id : allIDs)
                    changes.push_back(new FadeChangeRequest(id, curTime, curTime+sceneRep->fade));
                
                removeTime = curTime + sceneRep->fade;
            }
            
            sceneRep->clearContents(changes,removeTime);
            sceneReps.erase(it);
            delete sceneRep;
        }
    }
}

}
