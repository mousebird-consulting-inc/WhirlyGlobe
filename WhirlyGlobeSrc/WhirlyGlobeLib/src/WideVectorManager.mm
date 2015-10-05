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
#import "NSDictionary+Stuff.h"
#import "UIColor+Stuff.h"
#import "FlatMath.h"

using namespace WhirlyKit;
using namespace Eigen;

@implementation WhirlyKitWideVectorInfo

- (id)initWithDesc:(NSDictionary *)desc
{
    self = [super initWithDesc:desc];
    [self parseDesc:desc];
    
    return self;
}

- (void)parseDesc:(NSDictionary *)desc
{
    _color = [desc objectForKey:@"color" checkType:[UIColor class] default:[UIColor whiteColor]];
    _width = [desc floatForKey:@"width" default:2.0];
    _coordType = (WhirlyKit::WideVectorCoordsType)[desc enumForKey:@"wideveccoordtype" values:@[@"real",@"screen"] default:WideVecCoordScreen];
    _joinType = (WhirlyKit::WideVectorLineJoinType)[desc enumForKey:@"wideveclinejointype" values:@[@"miter",@"round",@"bevel"] default:WideVecMiterJoin];
    _capType = (WhirlyKit::WideVectorLineCapType)[desc enumForKey:@"wideveclinecaptype" values:@[@"butt",@"round",@"square"] default:WideVecButtCap];
    _texID = [desc intForKey:@"texture" default:EmptyIdentity];
    _repeatSize = [desc floatForKey:@"repeatSize" default:(_coordType == WideVecCoordScreen ? 32 : 6371000.0 / 20)];
    _miterLimit = [desc floatForKey:@"miterLimit" default:2.0];
    _texSnap = [desc boolForKey:@"texsnap" default:false];
}

@end

namespace WhirlyKit
{

class WideVectorBuilder
{
public:
    WideVectorBuilder(WhirlyKitWideVectorInfo *vecInfo,const Point3d &localCenter,const Point3d &dispCenter,const RGBAColor inColor,CoordSystemDisplayAdapter *coordAdapter)
    : vecInfo(vecInfo), angleCutoff(DegToRad(30.0)), texOffset(0.0), edgePointsValid(false), coordAdapter(coordAdapter), localCenter(localCenter), dispCenter(dispCenter)
    {
//        color = [vecInfo.color asRGBAColor];
        color = inColor;
    }
    
    // Intersect widened lines for the miter case
    bool intersectWideLines(const Point3d &p0,const Point3d &p1,const Point3d &p2,const Point3d &n0,const Point3d &n1,Point3d &iPt,double &t0,double &t1)
    {
        Point2d p10(p1.x()-p0.x(),p1.y()-p0.y());
        Point2d p21(p2.x()-p1.x(),p2.y()-p1.y());
        Point2d pn0(p0.x()+n0.x(),p0.y()+n0.y());
        Point2d pn1(p1.x()+n1.x(),p1.y()+n1.y());
        
        // Choose the form of the equation based on the size of this denominator
        double num, denom;
        if (std::abs(p10.x()) > std::abs(p10.y()))
        {
            double termA = p10.y()/p10.x();
            denom = p21.y() - p21.x() * termA;
            num = (pn1.x() - pn0.x())*termA + pn0.y()-pn1.y();
        } else {
            double termA = p10.x()/p10.y();
            denom = p21.y()*termA-p21.x();
            num = pn1.x() - pn0.x() + (pn0.y() - pn1.y())*termA;
        }
        if (denom == 0.0)
            return false;
        
        t1 = num/denom;
        iPt = (p2-p1) * t1 + p1 + n1;
        
        if (std::abs(p10.x()) > std::abs(p10.y()))
            t0 = (p21.x() * t1 + pn1.x() - pn0.x())/p10.x();
        else
            t0 = (p21.y() * t1 + pn1.y() - pn0.y())/p10.y();
                
        return true;
    }
    
    // Straight up 2D line intersection.  Z is ignred until the end.
    bool intersectLinesIn2D(const Point3d &p1,const Point3d &p2,const Point3d &p3,const Point3d &p4,Point3d *iPt)
    {
        float denom = (p1.x()-p2.x())*(p3.y()-p4.y()) - (p1.y() - p2.y())*(p3.x() - p4.x());
        if (denom == 0.0)
            return false;
        
        float termA = (p1.x()*p2.y() - p1.y()*p2.x());
        float termB = (p3.x() * p4.y() - p3.y() * p4.x());
        iPt->x() = ( termA * (p3.x() - p4.x()) - (p1.x() - p2.x()) * termB)/denom;
        iPt->y() = ( termA * (p3.y() - p4.y()) - (p1.y() - p2.y()) * termB)/denom;
        iPt->z() = 0.0;
        
        return true;
    }
    
    // Intersect lines using the origin,direction form. Just a 2D intersection
    bool intersectLinesDir(const Point3d &aOrg,const Point3d &aDir,const Point3d &bOrg,const Point3d &bDir,Point3d &iPt)
    {
        // Choose the form of the equation based on the size of this denominator
        double num, denom;
        if (std::abs(aDir.x()) > std::abs(aDir.y()))
        {
            double termA = aDir.y()/aDir.x();
            denom = bDir.y() - bDir.x() * termA;
            num = (bOrg.x() - aOrg.x())*termA + aOrg.y()-bOrg.y();
        } else {
            double termA = aDir.x()/aDir.y();
            denom = bDir.y()*termA-bDir.x();
            num = bOrg.x() - aOrg.x() + (aOrg.y() - bOrg.y())*termA;
        }
        if (denom == 0.0)
            return false;
        
        double t1 = num/denom;
        iPt = bDir * t1 + bOrg;
        
        return true;
    }
    
    // Add a rectangle to the drawable
    void addRect(BasicDrawable *drawable,Point3d *corners,TexCoord *texCoords,const Point3d &up,const RGBAColor &thisColor)
    {
        int startPt = drawable->getNumPoints();

        for (unsigned int vi=0;vi<4;vi++)
        {
            Point3d dispPt = corners[vi];
            drawable->addPoint(dispPt);
            if (vecInfo.texID != EmptyIdentity)
                drawable->addTexCoord(0, texCoords[vi]);
            drawable->addNormal(up);
//            drawable->addColor(thisColor);
        }
        
        drawable->addTriangle(BasicDrawable::Triangle(startPt+0,startPt+1,startPt+3));
        drawable->addTriangle(BasicDrawable::Triangle(startPt+1,startPt+2,startPt+3));
    }
    
    // Add a rectangle to the wide drawable
    void addWideRect(WideVectorDrawable *drawable,Point3d *corners,const Point3d &pa,const Point3d &pb,TexCoord *texCoords,const Point3d &up,const RGBAColor &thisColor)
    {
        int startPt = drawable->getNumPoints();
        
        for (unsigned int vi=0;vi<4;vi++)
        {
            drawable->addPoint(vi < 2 ? pa : pb);
            drawable->addDir(corners[vi]);
            if (vecInfo.texID != EmptyIdentity)
                drawable->addTexCoord(0, texCoords[vi]);
            drawable->addNormal(up);
            drawable->addMaxLen(0.0);
//            drawable->addColor(thisColor);
        }
        
        drawable->addTriangle(BasicDrawable::Triangle(startPt+0,startPt+1,startPt+3));
        drawable->addTriangle(BasicDrawable::Triangle(startPt+1,startPt+2,startPt+3));
    }
    
    // Add a triangle to the drawable
    void addTri(BasicDrawable *drawable,Point3d *corners,TexCoord *texCoords,const Point3d &up,const RGBAColor &thisColor)
    {
        int startPt = drawable->getNumPoints();
        
        for (unsigned int vi=0;vi<3;vi++)
        {
            drawable->addPoint(corners[vi]);
            if (vecInfo.texID != EmptyIdentity)
                drawable->addTexCoord(0, texCoords[vi]);
            drawable->addNormal(up);
//            drawable->addColor(thisColor);
        }
        
        drawable->addTriangle(BasicDrawable::Triangle(startPt+0,startPt+1,startPt+2));
    }
    
    // Add a triangle to the wide drawable
    void addWideTri(WideVectorDrawable *drawable,Point3d *corners,const Point3d &org,TexCoord *texCoords,float len,const Point3d &up,const RGBAColor &thisColor)
    {
        int startPt = drawable->getNumPoints();

        for (unsigned int vi=0;vi<3;vi++)
        {
            drawable->addPoint(org);
            drawable->addDir(corners[vi]);
            drawable->addMaxLen(len);
            
            if (vecInfo.texID != EmptyIdentity)
                drawable->addTexCoord(0, texCoords[vi]);
            drawable->addNormal(up);
//            drawable->addColor(thisColor);
        }
        
        drawable->addTriangle(BasicDrawable::Triangle(startPt+0,startPt+1,startPt+2));
    }
    
    // Build the polygons for a widened line segment
    void buildPolys(const Point3d *pa,const Point3d *pb,const Point3d *pc,const Point3d &up,BasicDrawable *drawable)
    {
        WideVectorDrawable *wideDrawable = dynamic_cast<WideVectorDrawable *>(drawable);
        
        double texLen = (*pb-*pa).norm();
        double texJoinLen = 0;
        // Degenerate segment
        if (texLen == 0.0)
            return;
        if (vecInfo.coordType == WideVecCoordReal)
            texLen /= vecInfo.repeatSize;
        
        // Next segment is degenerate
        if (pc)
        {
            if ((*pc-*pb).norm() == 0.0)
                pc = NULL;
        }

        double calcScale = (vecInfo.coordType == WideVecCoordReal ? 1.0 : 1/EarthRadius);

        // We need the normal (with respect to the line), and its inverse
        // These are half, for half the width
        Point3d norm0 = (*pb-*pa).cross(up);
        norm0.normalize();
        norm0 /= 2.0;
        Point3d revNorm0 = norm0 * -1.0;
        
        Point3d norm1(0,0,0),revNorm1(0,0,0);
        if (pc)
        {
            norm1 = (*pc-*pb).cross(up);
            norm1.normalize();
            norm1 /= 2.0;
            revNorm1 = norm1 * -1.0;
        }
        
        if (vecInfo.coordType == WideVecCoordReal)
        {
            norm0 *= vecInfo.width;
            norm1 *= vecInfo.width;
            revNorm0 *= vecInfo.width;
            revNorm1 *= vecInfo.width;
        }

        Point3d paLocal = *pa-dispCenter;
        Point3d pbLocal = *pb-dispCenter;
        Point3d pbLocalAdj = pbLocal;

        // Look for valid starting points.  If they're not there, make some simple ones
        if (!edgePointsValid)
        {
            if (vecInfo.coordType == WideVecCoordReal)
            {
                e0 = paLocal + revNorm0;
                e1 = paLocal + norm0;
            } else {
                e0 = paLocal + revNorm0*calcScale;
                e1 = paLocal + norm0*calcScale;
            }
            centerAdj = paLocal;
        }
        
        RGBAColor thisColor = color;
//        float scale = drand48() / 2 + 0.5;
//        thisColor.r *= scale;
//        thisColor.g *= scale;
//        thisColor.b *= scale;
        
        // Calculate points for the expanded linear
        Point3d corners[4];
        TexCoord texCoords[4];
        
        Point3d rPt,lPt;
        Point3d pcLocal = (pc ? *pc-dispCenter: Point3d(0,0,0));
        Point3d dirA = (paLocal-pbLocal).normalized();
        double lenA = (paLocal-pbLocal).norm();
        double lenB = 0.0;
        Point3d dirB;
        
        // Figure out which way the bend goes and calculation intersection points
        double t0l,t1l,t0r,t1r;
        bool iPtsValid = false;
        if (pc)
        {
            // Compare the angle between the two segments.
            // We want to catch when the data folds back on itself.
            dirB = (pcLocal-pbLocal).normalized();
            lenB = (pcLocal-pbLocal).norm();
            double dot = dirA.dot(dirB);
            if (dot > -0.99999998476 && dot < 0.99999998476)
                if (intersectWideLines(paLocal,pbLocal,pcLocal,norm0*calcScale,norm1*calcScale,rPt,t0r,t1r) &&
                    intersectWideLines(paLocal,pbLocal,pcLocal,revNorm0*calcScale,revNorm1*calcScale,lPt,t0l,t1l))
                    iPtsValid = true;
        }
        
        // Points from the last round
        corners[0] = e0;
        corners[1] = e1;
        
        Point3d next_e0,next_e1,next_e0_dir,next_e1_dir;
        if (iPtsValid)
        {
            // Bending right
            if (t0l > 1.0)
            {
                // Make sure we didn't exceed the length of either segment
                if (t0r > 0.0 && t0r < 1.0 && t1r > 0.0 && t1r < 1.0)
                {
                    if (vecInfo.coordType == WideVecCoordReal)
                    {
                        corners[2] = rPt;
                        corners[3] = rPt + revNorm0 * 2;
                        next_e0 = rPt + revNorm1 * 2;
                        next_e1 = corners[2];
                    } else {
                        corners[2] = rPt;
                        corners[3] = rPt + revNorm0 * calcScale * 2;
                        
                        next_e0 = rPt + revNorm1 * calcScale * 2;
                        next_e1 = corners[2];
                    }
                } else
                    iPtsValid = false;
            } else {
                // Bending left
                // Make sure we didn't exceed the length of either segment
                if (t0l > 0.0 && t0l < 1.0 && t1l > 0.0 && t1l < 1.0)
                {
                    if (vecInfo.coordType == WideVecCoordReal)
                    {
                        corners[2] = lPt + norm0 * 2;
                        corners[3] = lPt;
                        next_e0 = corners[3];
                        next_e1 = lPt + norm1 * 2;
                    } else {
                        corners[2] = lPt + norm0 * calcScale * 2;
                        corners[3] = lPt;

                        next_e0 = lPt;
                        next_e1 = lPt + norm1 * calcScale * 2;
                    }
                } else
                    iPtsValid = false;
            }
        }
        
        if (!iPtsValid)
        {
            if (vecInfo.coordType == WideVecCoordReal)
            {
                corners[2] = pbLocal + norm0;
                corners[3] = pbLocal + revNorm0;
                next_e0 = corners[3];
                next_e1 = corners[2];
            } else {
                corners[2] = pbLocal + norm0 * calcScale;
                corners[3] = pbLocal + revNorm0 * calcScale;
                next_e0 = corners[3];
                next_e1 = corners[2];
            }
            edgePointsValid = false;
        } else
            edgePointsValid = true;
        
        texCoords[0] = TexCoord(0.0,texOffset);
        texCoords[1] = TexCoord(1.0,texOffset);
        texCoords[2] = TexCoord(1.0,texOffset+texLen);
        texCoords[3] = TexCoord(0.0,texOffset+texLen);
        
        double minSegLen = 0.0;
        if (vecInfo.coordType == WideVecCoordScreen)
            minSegLen = std::max(lenA,lenB);
        
        // Make an explicit join
        Point3d triVerts[3];
        TexCoord triTex[3];
        if (iPtsValid)
        {
            double len = 0.0;
            WideVectorLineJoinType joinType = vecInfo.joinType;
            
            // We may need to switch to a bevel join if miter is too extreme
            if (joinType == WideVecMiterJoin)
            {
                // Bending right
                if (t0l > 1.0)
                {
                    // Measure the distance from the left point to the middle
                    len = (lPt-pbLocal).norm()/calcScale;
                } else {
                    // Bending left
                    len = (rPt-pbLocal).norm()/calcScale;
                }
                
                if (vecInfo.coordType == WideVecCoordReal)
                {
                    if (2*len/vecInfo.width > vecInfo.miterLimit)
                        joinType = WideVecBevelJoin;
                } else {
                    if (2*len > vecInfo.miterLimit)
                        joinType = WideVecBevelJoin;
                }
            }
            
            switch (joinType)
            {
                case WideVecMiterJoin:
                {
                    // Bending right
                    if (t0l > 1.0)
                    {
                        // Build two triangles to join up to the middle
                        double texJoinLens[2];
                        texJoinLens[0] = (lPt-corners[3]).norm()/2.0;
                        triTex[0] = TexCoord(0.0,texOffset+texLen);
                        triTex[1] = TexCoord(1.0,texOffset+texLen);
                        triTex[2] = TexCoord(0.0,texOffset+texLen+texJoinLens[0]);
                        if (vecInfo.coordType == WideVecCoordReal)
                        {
                            triVerts[0] = corners[3];
                            triVerts[1] = rPt;
                            triVerts[2] = lPt;
                            addTri(drawable,triVerts,triTex,up,thisColor);
                        } else {
                            triVerts[0] = (corners[3]-pbLocal)/calcScale;
                            triVerts[1] = (rPt-pbLocal)/calcScale;
                            triVerts[2] = (lPt-pbLocal)/calcScale;
                            addWideTri(wideDrawable,triVerts,pbLocal,triTex,minSegLen,up,thisColor);
                        }
                        texJoinLens[1] = (next_e0-lPt).norm()/2.0;
                        triTex[0] = TexCoord(0.0,texOffset+texLen+texJoinLens[0]);
                        triTex[1] = TexCoord(1.0,texOffset+texLen+texJoinLens[0]);
                        triTex[2] = TexCoord(0.0,texOffset+texLen+texJoinLens[0]+texJoinLens[1]);
                        if (vecInfo.coordType == WideVecCoordReal)
                        {
                            triVerts[0] = lPt;
                            triVerts[1] = rPt;
                            triVerts[2] = next_e0;
                            addTri(drawable,triVerts,triTex,up,thisColor);
                        } else {
                            triVerts[0] = (lPt-pbLocal)/calcScale;
                            triVerts[1] = (rPt-pbLocal)/calcScale;
                            triVerts[2] = (next_e0-pbLocal)/calcScale;
                            addWideTri(wideDrawable,triVerts,pbLocal,triTex,minSegLen,up,thisColor);
                        }
                        texJoinLen = texJoinLens[0] + texJoinLens[1];
                    } else {
                        // Bending left
                        double texJoinLens[2];
                        texJoinLens[0] = (rPt-corners[2]).norm()/2.0;
                        triTex[0] = TexCoord(0.0,texOffset+texLen);
                        triTex[1] = TexCoord(1.0,texOffset+texLen);
                        triTex[2] = TexCoord(1.0,texOffset+texLen+texJoinLens[0]);
                        if (vecInfo.coordType == WideVecCoordReal)
                        {
                            triVerts[0] = lPt;
                            triVerts[1] = corners[2];
                            triVerts[2] = rPt;
                            addTri(drawable,triVerts,triTex,up,thisColor);
                        } else {
                            triVerts[0] = (lPt-pbLocal)/calcScale;
                            triVerts[1] = (corners[2]-pbLocal)/calcScale;
                            triVerts[2] = (rPt-pbLocal)/calcScale;
                            addWideTri(wideDrawable,triVerts,pbLocal,triTex,minSegLen,up,thisColor);
                        }
                        texJoinLens[1] = (next_e1-rPt).norm()/2.0;
                        triTex[0] = TexCoord(0.0,texOffset+texLen+texJoinLens[0]);
                        triTex[1] = TexCoord(1.0,texOffset+texLen+texJoinLens[0]);
                        triTex[2] = TexCoord(1.0,texOffset+texLen+texJoinLens[0]+texJoinLens[1]);
                        if (vecInfo.coordType == WideVecCoordReal)
                        {
                            triVerts[0] = lPt;
                            triVerts[1] = rPt;
                            triVerts[2] = next_e1;
                            addTri(drawable,triVerts,triTex,up,thisColor);
                        } else {
                            triVerts[0] = (lPt-pbLocal)/calcScale;
                            triVerts[1] = (rPt-pbLocal)/calcScale;
                            triVerts[2] = (next_e1-pbLocal)/calcScale;
                            addWideTri(wideDrawable,triVerts,pbLocal,triTex,minSegLen,up,thisColor);
                        }
                        texJoinLen = texJoinLens[0] + texJoinLens[1];
                    }
                }
                    break;
                case WideVecBevelJoin:
                {
                    // Bending right
                    if (t0l > 1.0)
                    {
                        // lPt1 is a point in the middle of the prospective bevel
                        Point3d lNorm = (lPt-pbLocal).normalized();
                        Point3d lPt1 = pbLocal + lNorm * calcScale * (vecInfo.coordType == WideVecCoordReal ? vecInfo.width : 1.0);
                        Point3d iNorm = up.cross(lNorm);
//                        Point3d juncCtr = (rPt+lPt1)/2.0;
//                        pbLocalAdj = juncCtr;
                        
                        // Find the intersection points with the edges along the left side
                        Point3d li0,li1;
                        if (intersectLinesDir(lPt1,iNorm,corners[0],pbLocal-paLocal,li0) &&
                            intersectLinesDir(lPt1,iNorm,next_e0,pcLocal-pbLocal,li1))
                        {
                            double texLens[3];
                            // Form three triangles for this junction
                            texLens[0] = (li0-corners[3]).norm()/2.0;
                            triTex[0] = TexCoord(0.0,texOffset+texLen);
                            triTex[1] = TexCoord(1.0,texOffset+texLen);
                            triTex[2] = TexCoord(0.0,texOffset+texLen+texLens[0]);
                            if (vecInfo.coordType == WideVecCoordReal)
                            {
                                triVerts[0] = corners[3];
                                triVerts[1] = rPt;
                                triVerts[2] = li0;
                                addTri(drawable,triVerts,triTex,up,thisColor);
                            } else {
                                triVerts[0] = (corners[3]-pbLocalAdj)/calcScale;
                                triVerts[1] = (rPt-pbLocalAdj)/calcScale;
                                triVerts[2] = (li0-pbLocalAdj)/calcScale;
                                addWideTri(wideDrawable,triVerts,pbLocalAdj,triTex,minSegLen,up,thisColor);
                            }
                            texLens[1] = (li1-li0).norm()/2.0;
                            triTex[0] = TexCoord(0.0,texOffset+texLen+texLens[0]);
                            triTex[1] = TexCoord(1.0,texOffset+texLen+texLens[0]);
                            triTex[2] = TexCoord(0.0,texOffset+texLen+texLens[0]+texLens[1]);
                            if (vecInfo.coordType == WideVecCoordReal)
                            {
                                triVerts[0] = li0;
                                triVerts[1] = rPt;
                                triVerts[2] = li1;
                                addTri(drawable,triVerts,triTex,up,thisColor);
                            } else {
                                triVerts[0] = (li0-pbLocalAdj)/calcScale;
                                triVerts[1] = (rPt-pbLocalAdj)/calcScale;
                                triVerts[2] = (li1-pbLocalAdj)/calcScale;
                                addWideTri(wideDrawable,triVerts,pbLocalAdj,triTex,minSegLen,up,thisColor);
                            }
                            texLens[2] = (next_e0-li1).norm()/2.0;
                            triTex[0] = TexCoord(0.0,texOffset+texLen+texLens[0]+texLens[1]);
                            triTex[1] = TexCoord(1.0,texOffset+texLen+texLens[0]+texLens[1]);
                            triTex[2] = TexCoord(0.0,texOffset+texLen+texLens[0]+texLens[1]+texLens[2]);
                            if (vecInfo.coordType == WideVecCoordReal)
                            {
                                triVerts[0] = li1;
                                triVerts[1] = rPt;
                                triVerts[2] = next_e0;
                                addTri(drawable,triVerts,triTex,up,thisColor);
                            } else {
                                triVerts[0] = (li1-pbLocalAdj)/calcScale;
                                triVerts[1] = (rPt-pbLocalAdj)/calcScale;
                                triVerts[2] = (next_e0-pbLocalAdj)/calcScale;
                                addWideTri(wideDrawable,triVerts,pbLocalAdj,triTex,minSegLen,up,thisColor);
                            }
                            texJoinLen = texLens[0] + texLens[1] + texLens[2];
                        }
                    } else {
                        // Bending left
                        // rPt1 is a point in the middle of the prospective bevel
                        Point3d rNorm = (rPt-pbLocal).normalized();
                        Point3d rPt1 = pbLocal + rNorm * calcScale * (vecInfo.coordType == WideVecCoordReal ? vecInfo.width : 1.0);
                        Point3d iNorm = rNorm.cross(up);
//                        Point3d juncCtr = (lPt+rPt1)/2.0;
                        //                        pbLocalAdj = juncCtr;
                        
                        // Find the intersection points with the edges along the right side
                        Point3d ri0,ri1;
                        if (intersectLinesDir(rPt1,iNorm,corners[1], pbLocal-paLocal, ri0) &&
                            intersectLinesDir(rPt1,iNorm,next_e1,pcLocal-pbLocal,ri1))
                        {
                            double texLens[3];
                            // Form three triangles for this junction
                            texLens[0] = (ri0-corners[2]).norm()/2.0;
                            triTex[0] = TexCoord(0.0,texOffset+texLen);
                            triTex[1] = TexCoord(1.0,texOffset+texLen);
                            triTex[2] = TexCoord(1.0,texOffset+texLen+texLens[0]);
                            if (vecInfo.coordType == WideVecCoordReal)
                            {
                                triVerts[0] = lPt;
                                triVerts[1] = corners[2];
                                triVerts[2] = ri0;
                                addTri(drawable,triVerts,triTex,up,thisColor);
                            } else {
                                triVerts[0] = (lPt-pbLocalAdj)/calcScale;
                                triVerts[1] = (corners[2]-pbLocalAdj)/calcScale;
                                triVerts[2] = (ri0-pbLocalAdj)/calcScale;
                                addWideTri(wideDrawable,triVerts,pbLocalAdj,triTex,minSegLen,up,thisColor);
                            }
                            texLens[1] = (ri1-ri0).norm()/2.0;
                            triTex[0] = TexCoord(0.0,texOffset+texLen+texLens[0]);
                            triTex[1] = TexCoord(1.0,texOffset+texLen+texLens[0]);
                            triTex[2] = TexCoord(1.0,texOffset+texLen+texLens[0]+texLens[1]);
                            if (vecInfo.coordType == WideVecCoordReal)
                            {
                                triVerts[0] = lPt;
                                triVerts[1] = ri0;
                                triVerts[2] = ri1;
                                addTri(drawable,triVerts,triTex,up,thisColor);
                            } else {
                                triVerts[0] = (lPt-pbLocalAdj)/calcScale;
                                triVerts[1] = (ri0-pbLocalAdj)/calcScale;
                                triVerts[2] = (ri1-pbLocalAdj)/calcScale;
                                addWideTri(wideDrawable,triVerts,pbLocalAdj,triTex,minSegLen,up,thisColor);
                            }
                            texLens[2] = (next_e1-ri1).norm()/2.0;
                            triTex[0] = TexCoord(0.0,texOffset+texLen+texLens[0]+texLens[1]);
                            triTex[1] = TexCoord(1.0,texOffset+texLen+texLens[0]+texLens[1]);
                            triTex[2] = TexCoord(1.0,texOffset+texLen+texLens[0]+texLens[1]+texLens[2]);
                            if (vecInfo.coordType == WideVecCoordReal)
                            {
                                triVerts[0] = lPt;
                                triVerts[1] = ri1;
                                triVerts[2] = next_e1;
                                addTri(drawable,triVerts,triTex,up,thisColor);
                            } else {
                                triVerts[0] = (lPt-pbLocalAdj)/calcScale;
                                triVerts[1] = (ri1-pbLocalAdj)/calcScale;
                                triVerts[2] = (next_e1-pbLocalAdj)/calcScale;
                                addWideTri(wideDrawable,triVerts,pbLocalAdj,triTex,minSegLen,up,thisColor);
                            }
                            texJoinLen = texLens[0] + texLens[1] + texLens[2];
                        }
                    }
                }
                    break;
                case WideVecRoundJoin:
                    break;
            }
        }
        
        // Add the segment rectangle
        if (vecInfo.coordType == WideVecCoordReal)
        {
            addRect(drawable,corners,texCoords,up,thisColor);
        } else {
            // Run the offsets for the corners.
            Point3d cornerVecs[4];
            for (unsigned int ii=0;ii<4;ii++)
                cornerVecs[ii] = (corners[ii]-((ii < 2) ? centerAdj : pbLocalAdj))/calcScale;

            addWideRect(wideDrawable,cornerVecs,centerAdj,pbLocalAdj,texCoords,up,thisColor);
        }
        
        e0 = next_e0;
        e1 = next_e1;
        centerAdj = pbLocalAdj;
        texOffset += texLen+texJoinLen;
    }
    
    
    // Add a point to the widened linear we're building
    void addPoint(const Point3d &inPt,const Point3d &up,BasicDrawable *drawable)
    {
        // Compare with the last point, if it's the same, toss it
        if (!pts.empty() && pts.back() == inPt)
            return;
        
        pts.push_back(inPt);
        if (pts.size() >= 3)
        {
            const Point3d &pa = pts[pts.size()-3];
            const Point3d &pb = pts[pts.size()-2];
            const Point3d &pc = pts[pts.size()-1];
            buildPolys(&pa,&pb,&pc,up,drawable);
        }
        lastUp = up;
    }
    
    // Flush out any outstanding points
    void flush(BasicDrawable *drawable)
    {
        if (pts.size() >= 2)
        {
            const Point3d &pa = pts[pts.size()-2];
            const Point3d &pb = pts[pts.size()-1];
            buildPolys(&pa, &pb, NULL, lastUp, drawable);
        }
    }

    WhirlyKitWideVectorInfo *vecInfo;
    CoordSystemDisplayAdapter *coordAdapter;
    RGBAColor color;
    Point3d localCenter,dispCenter;
    double angleCutoff;
    
    double texOffset;

    std::vector<Point3d> pts;
    Point3d lastUp;
    
    bool edgePointsValid;
    Point3d e0,e1,centerAdj;
};

// Used to build up drawables
class WideVectorDrawableBuilder
{
public:
    WideVectorDrawableBuilder(Scene *scene,WhirlyKitWideVectorInfo *vecInfo)
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
        if (!drawable ||
            (drawable->getNumPoints()+ptCount > MaxDrawablePoints) ||
            (drawable->getNumTris()+triCount > MaxDrawableTriangles))
        {
            flush();
          
            if (vecInfo.coordType == WideVecCoordReal)
            {
                drawable = new BasicDrawable("WideVector");
            } else {
                WideVectorDrawable *wideDrawable = new WideVectorDrawable();
                drawable = wideDrawable;
                drawable->setProgram(vecInfo.programID);
                wideDrawable->setTexRepeat(vecInfo.repeatSize);
                wideDrawable->setLineWidth(vecInfo.width);
            }
//            drawMbr.reset();
            drawable->setType(GL_TRIANGLES);
            [vecInfo setupBasicDrawable:drawable];
            drawable->setColor([vecInfo.color asRGBAColor]);
            if (vecInfo.texID != EmptyIdentity)
                drawable->setTexId(0, vecInfo.texID);
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
    void addLinear(VectorRing &pts,const Point3d &up)
    {
        RGBAColor color = [vecInfo.color asRGBAColor];
//        color.r = random()%256;
//        color.g = random()%256;
//        color.b = random()%256;
//        color.a = 255;
        WideVectorBuilder vecBuilder(vecInfo,localCenter,dispCenter,color,coordAdapter);
        
        // Work through the segments
        for (unsigned int ii=0;ii<pts.size();ii++)
        {
            // Get the points in display space
            Point2f geoA = pts[ii];
            Point3d dispPa = coordAdapter->localToDisplay(coordSys->geographicToLocal3d(GeoCoord(geoA.x(),geoA.y())));
            
            // Get a drawable ready
            int ptCount = 5;
            int triCount = 4;
            BasicDrawable *thisDrawable = getDrawable(ptCount,triCount);
            drawMbr.addPoint(geoA);
            
            vecBuilder.addPoint(dispPa,up,thisDrawable);
        }

        vecBuilder.flush(drawable);
    }

    // Flush out the drawables
    WideVectorSceneRep *flush(ChangeSet &changes)
    {
        flush();
        
        if (drawables.empty())
            return NULL;
        
        WideVectorSceneRep *sceneRep = new WideVectorSceneRep();
        sceneRep->fade = vecInfo.fade;
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
    WhirlyKitWideVectorInfo *vecInfo;
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
    
SimpleIdentity WideVectorManager::addVectors(ShapeSet *shapes,NSDictionary *desc,ChangeSet &changes)
{
    WhirlyKitWideVectorInfo *vecInfo = [[WhirlyKitWideVectorInfo alloc] initWithDesc:desc];
    
    WideVectorDrawableBuilder builder(scene,vecInfo);
    
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
        VectorLinearRef lin = boost::dynamic_pointer_cast<VectorLinear>(*it);
        if (lin)
        {
            builder.addLinear(lin->pts,centerUp);
        }
    }
    
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
    
SimpleIdentity WideVectorManager::instanceVectors(SimpleIdentity vecID,NSDictionary *desc,ChangeSet &changes)
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
            if ([desc objectForKey:@"color"]) {
                RGBAColor newColor = [[desc objectForKey:@"color" checkType:[UIColor class] default:[UIColor whiteColor]] asRGBAColor];
                drawInst->setColor(newColor);
            }
            
            // Changed visibility
            if ([desc objectForKey:@"minVis"] || [desc objectForKey:@"maxVis"]) {
                float minVis = [desc floatForKey:@"minVis" default:DrawVisibleInvalid];
                float maxVis = [desc floatForKey:@"maxVis" default:DrawVisibleInvalid];
                drawInst->setVisibleRange(minVis, maxVis);
            }
            
            // Changed line width
            if ([desc objectForKey:@"width"]) {
                float lineWidth = [desc floatForKey:@"width" default:1.0];
                drawInst->setLineWidth(lineWidth);
            }
            
            // Changed draw priority
            if ([desc objectForKey:@"drawPriority"] || [desc objectForKey:@"priority"]) {
                int priority = [desc intForKey:@"drawPriority" default:0];
                // This looks like an old bug
                priority = [desc intForKey:@"priority" default:priority];
                drawInst->setDrawPriority(priority);
            }
            
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
        NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
        if (it != sceneReps.end())
        {
            WideVectorSceneRep *sceneRep = *it;
            
            if (sceneRep->fade > 0.0)
            {
                SimpleIDSet allIDs = sceneRep->drawIDs;
                allIDs.insert(sceneRep->instIDs.begin(),sceneRep->instIDs.end());
                for (SimpleIDSet::iterator it = allIDs.begin();
                     it != allIDs.end(); ++it)
                    changes.push_back(new FadeChangeRequest(*it, curTime, curTime+sceneRep->fade));
                
                __block NSObject * __weak thisCanary = canary;

                // Spawn off the deletion for later
                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, sceneRep->fade * NSEC_PER_SEC),
                               scene->getDispatchQueue(),
                               ^{
                                   if (thisCanary)
                                   {
                                       SimpleIDSet theIDs;
                                       theIDs.insert(sceneRep->getId());
                                       ChangeSet delChanges;
                                       removeVectors(theIDs, delChanges);
                                       scene->addChangeRequests(delChanges);
                                   }
                               }
                               );
                
                sceneRep->fade = 0.0;
            } else {
                (*it)->clearContents(changes);
                sceneReps.erase(it);
                delete sceneRep;
            }
        }
    }
    
    pthread_mutex_unlock(&vecLock);
}

}
