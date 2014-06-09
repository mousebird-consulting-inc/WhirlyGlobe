/*
 *  WideVectorManager.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/29/14.
 *  Copyright 2011-2014 mousebird consulting. All rights reserved.
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

- (void)parseDesc:(NSDictionary *)desc
{
    _color = [desc objectForKey:@"color" checkType:[UIColor class] default:[UIColor whiteColor]];
    _minVis = [desc floatForKey:@"minVis" default:DrawVisibleInvalid];
    _maxVis = [desc floatForKey:@"maxVis" default:DrawVisibleInvalid];
    _fade = [desc floatForKey:@"fade" default:0.0];
    _drawPriority = [desc intForKey:@"drawPriority" default:0];
    _enable = [desc boolForKey:@"enable" default:true];
    _shader = [desc intForKey:@"shader" default:EmptyIdentity];
    _width = [desc floatForKey:@"width" default:2.0];
    _coordType = (WhirlyKit::WideVectorCoordsType)[desc enumForKey:@"wideveccoordtype" values:@[@"real",@"screen"] default:WideVecCoordScreen];
    _joinType = (WhirlyKit::WideVectorLineJoinType)[desc enumForKey:@"wideveclinejointype" values:@[@"miter",@"round",@"bevel"] default:WideVecMiterJoin];
    _capType = (WhirlyKit::WideVectorLineCapType)[desc enumForKey:@"wideveclinecaptype" values:@[@"butt",@"round",@"square"] default:WideVecButtCap];
    _texID = [desc intForKey:@"texture" default:EmptyIdentity];
    _repeatSize = [desc floatForKey:@"repeatSize" default:6371000.0 / 20];
    _miterLimit = [desc floatForKey:@"miterLimit" default:2.0];
}

@end

namespace WhirlyKit
{

class WideVectorBuilder
{
public:
    WideVectorBuilder(WhirlyKitWideVectorInfo *vecInfo,const Point3d &center,const RGBAColor inColor)
    : vecInfo(vecInfo), angleCutoff(DegToRad(30.0)), texOffset(0.0), center(center), edgePointsValid(false)
    {
//        color = [vecInfo.color asRGBAColor];
        color = inColor;
    }
    
    // Intersect widened lines for the miter case
    bool intersectWideLines(const Point3d &p0,const Point3d &p1,const Point3d &p2,const Point3d &n0,const Point3d &n1,Point3d &iPt)
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
        
        double t1 = num/denom;
        iPt = (p2-p1) * t1 + p1 + n1;
        
        return true;
    }
    
    // Build the polygons for a widened line segment
    void buildPolys(const Point3d *pa,const Point3d *pb,const Point3d *pc,const Point3d &up,BasicDrawable *drawable)
    {
        double texLen = (*pb-*pa).norm();
        texLen *= vecInfo.repeatSize;

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
        
        // Look for valid starting points.  If they're not there, make some simple ones
        if (!edgePointsValid)
        {
            if (vecInfo.coordType == WideVecCoordReal)
            {
                e0 = *pa + revNorm0 - center;
                e1 = *pa + norm0 - center;
            } else {
                e0 = revNorm0;
                e1 = norm0;
            }
        }
        
        RGBAColor thisColor = color;
        // Note: Debugging
//        float scale = drand48() / 2 + 0.5;
//        thisColor.r *= scale;
//        thisColor.g *= scale;
//        thisColor.b *= scale;

        int startPt = drawable->getNumPoints();
        if (vecInfo.coordType == WideVecCoordReal)
        {
            // Calculate points for the expanded linear
            Point3d corners[4];
            TexCoord texCoords[4];
            
            Point3d rPt,lPt;
            Point3d paLocal = *pa-center;
            Point3d pbLocal = *pb-center;
            Point3d pcLocal = (pc ? *pc-center: Point3d(0,0,0));
            switch (vecInfo.joinType)
            {
                case WideVecMiterJoin:
                {
                    bool iPtsValid = false;
                    if (pc)
                    {
                        if (intersectWideLines(paLocal,pbLocal,pcLocal,norm0,norm1,rPt) &&
                            intersectWideLines(paLocal,pbLocal,pcLocal,revNorm0,revNorm1, lPt))
                            iPtsValid = true;
                    }
                    
                    corners[0] = e0;
                    corners[1] = e1;
                    if (!iPtsValid)
                    {
                        rPt = pbLocal + norm0;
                        lPt = pbLocal + revNorm0;
                    }
                }
                    break;
                case WideVecBevelJoin:
                    break;
                case WideVecRoundJoin:
                    break;
            }

            corners[2] = rPt;
            corners[3] = lPt;
            
            double rStartT=0.0,lStartT=0.0;
            double rEndT=1.0,lEndT=1.0;
            ClosestPointOnLineSegment(Point2d(paLocal.x(),paLocal.y()), Point2d(pbLocal.x(),pbLocal.y()), Point2d(e1.x(),e1.y()), rStartT);
            ClosestPointOnLineSegment(Point2d(paLocal.x(),paLocal.y()), Point2d(pbLocal.x(),pbLocal.y()), Point2d(e0.x(),e0.y()), lStartT);
            ClosestPointOnLineSegment(Point2d(paLocal.x(),paLocal.y()), Point2d(pbLocal.x(),pbLocal.y()), Point2d(rPt.x(),rPt.y()), rEndT);
            ClosestPointOnLineSegment(Point2d(paLocal.x(),paLocal.y()), Point2d(pbLocal.x(),pbLocal.y()), Point2d(lPt.x(),lPt.y()), lEndT);
            
            texCoords[0] = TexCoord(0.0,texOffset+lStartT*texLen);
            texCoords[1] = TexCoord(1.0,texOffset+rStartT*texLen);
            texCoords[2] = TexCoord(1.0,texOffset+texLen*rEndT);
            texCoords[3] = TexCoord(0.0,texOffset+texLen*lEndT);
            
            for (unsigned int vi=0;vi<4;vi++)
            {
                drawable->addPoint(corners[vi]);
                if (vecInfo.texID != EmptyIdentity)
                    drawable->addTexCoord(0, texCoords[vi]);
                drawable->addNormal(up);
                drawable->addColor(thisColor);
            }
            
            drawable->addTriangle(BasicDrawable::Triangle(startPt+0,startPt+1,startPt+3));
            drawable->addTriangle(BasicDrawable::Triangle(startPt+1,startPt+2,startPt+3));
            
            e0 = corners[3];
            e1 = corners[2];
            edgePointsValid = true;
        } else {
            WideVectorDrawable *wideDrawable = (WideVectorDrawable *)drawable;
            
            Point3d dirR,dirL;
            Point3d paAdj = *pa-center,pbAdj = *pb-center;
            Point3d pcAdj = (pc ? *pc-center : Point3d(0,0,0));
            switch (vecInfo.joinType)
            {
                case WideVecMiterJoin:
                {
                    bool iPtsValid = false;
                    Point3d rPt,lPt;
                    if (pc)
                    {
                        if (intersectWideLines(paAdj,pbAdj,pcAdj,norm0/EarthRadius,norm1/EarthRadius,rPt) &&
                            intersectWideLines(paAdj,pbAdj,pcAdj,revNorm0/EarthRadius,revNorm1/EarthRadius,lPt))
                            iPtsValid = true;
                    }

                    if (iPtsValid)
                    {
                        // We need vectors (with length), not actual points
                        dirR = (rPt-pbAdj) * EarthRadius;
                        dirL = (lPt-pbAdj) * EarthRadius;
                    } else {
                        dirR = norm0;  dirL = revNorm0;
                    }
                }
                    break;
                case WideVecBevelJoin:
                    dirR = norm0;  dirL = revNorm0;
                    break;
                case WideVecRoundJoin:
                    dirR = norm0;  dirL = revNorm0;
                    break;
            }
            
            // Note: This doesn't quite work.  Still see a bit of pulling in textures.
            double rStartT=0.0,lStartT=0.0;
            double rEndT=1.0,lEndT=1.0;
            Point3d e1Real = (paAdj + e1/EarthRadius);
            Point3d e0Real = (paAdj + e0/EarthRadius);
            Point3d rPtReal = (pbAdj + dirR/EarthRadius);
            Point3d lPtReal = (pbAdj + dirL/EarthRadius);
            ClosestPointOnLineSegment(Point2d(paAdj.x(),paAdj.y()), Point2d(pbAdj.x(),pbAdj.y()), Point2d(e1Real.x(),e1Real.y()), rStartT);
            ClosestPointOnLineSegment(Point2d(paAdj.x(),paAdj.y()), Point2d(pbAdj.x(),pbAdj.y()), Point2d(e0Real.x(),e0Real.y()), lStartT);
            ClosestPointOnLineSegment(Point2d(paAdj.x(),paAdj.y()), Point2d(pbAdj.x(),pbAdj.y()), Point2d(rPtReal.x(),rPtReal.y()), rEndT);
            ClosestPointOnLineSegment(Point2d(paAdj.x(),paAdj.y()), Point2d(pbAdj.x(),pbAdj.y()), Point2d(lPtReal.x(),lPtReal.y()), lEndT);
            
            TexCoord texCoords[4];
            texCoords[0] = TexCoord(0.0,texOffset+lStartT*texLen);
            texCoords[1] = TexCoord(1.0,texOffset+rStartT*texLen);
            texCoords[2] = TexCoord(1.0,texOffset+texLen*rEndT);
            texCoords[3] = TexCoord(0.0,texOffset+texLen*lEndT);

            // Add the "expanded" linear
            
            wideDrawable->addPoint(paAdj);
            if (vecInfo.texID != EmptyIdentity)
                wideDrawable->addTexCoord(0, texCoords[0]);
            wideDrawable->addNormal(up);
            wideDrawable->addDir(e0);
            wideDrawable->addColor(thisColor);
            
            wideDrawable->addPoint(paAdj);
            if (vecInfo.texID != EmptyIdentity)
                wideDrawable->addTexCoord(0, texCoords[1]);
            wideDrawable->addNormal(up);
            wideDrawable->addDir(e1);
            wideDrawable->addColor(thisColor);
            
            wideDrawable->addPoint(pbAdj);
            if (vecInfo.texID != EmptyIdentity)
                wideDrawable->addTexCoord(0, texCoords[2]);
            wideDrawable->addNormal(up);
            wideDrawable->addDir(dirR);
            wideDrawable->addColor(thisColor);
            
            wideDrawable->addPoint(pbAdj);
            if (vecInfo.texID != EmptyIdentity)
                wideDrawable->addTexCoord(0, texCoords[3]);
            wideDrawable->addNormal(up);
            wideDrawable->addDir(dirL);
            wideDrawable->addColor(thisColor);
            
            wideDrawable->addTriangle(BasicDrawable::Triangle(startPt+0,startPt+1,startPt+3));
            wideDrawable->addTriangle(BasicDrawable::Triangle(startPt+1,startPt+2,startPt+3));
            
            e0 = dirL;
            e1 = dirR;
            edgePointsValid = true;
        }
        
        texOffset += texLen;
    }
    
    // Add a point to the widened linear we're building
    void addPoint(const Point3d &inPt,const Point3d &up,BasicDrawable *drawable)
    {
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
    RGBAColor color;
    Point3d center;
    double angleCutoff;
    
    double texOffset;

    std::vector<Point3d> pts;
    Point3d lastUp;
    
    bool edgePointsValid;
    Point3d e0,e1;
};

// Used to build up drawables
class WideVectorDrawableBuilder
{
public:
    WideVectorDrawableBuilder(Scene *scene,WhirlyKitWideVectorInfo *vecInfo)
    : scene(scene), vecInfo(vecInfo), drawable(NULL), centerValid(false)
    {
        coordAdapter = scene->getCoordAdapter();
        coordSys = coordAdapter->getCoordSystem();
    }
    
    // Center to use for drawables we create
    void setCenter(const Point3d &newCenter)
    {
        centerValid = true;
        center = newCenter;
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
                drawable->setProgram(vecInfo.shader);
                wideDrawable->setWidth(vecInfo.width);
            }
//            drawMbr.reset();
            drawable->setType(GL_TRIANGLES);
            drawable->setOnOff(vecInfo.enable);
            drawable->setColor([vecInfo.color asRGBAColor]);
            drawable->setDrawPriority(vecInfo.drawPriority);
            drawable->setVisibleRange(vecInfo.minVis,vecInfo.maxVis);
            if (vecInfo.texID != EmptyIdentity)
                drawable->setTexId(0, vecInfo.texID);
            if (centerValid)
            {
                Eigen::Affine3d trans(Eigen::Translation3d(center.x(),center.y(),center.z()));
                Matrix4d transMat = trans.matrix();
                drawable->setMatrix(&transMat);
            }
        }
        
        return drawable;
    }
    
    // Add the points for a linear
    void addLinear(VectorRing &pts)
    {
        // Note: Debugging
        RGBAColor color = [vecInfo.color asRGBAColor];
//        color.r = random()%256;
//        color.g = random()%256;
//        color.b = random()%256;
//        color.a = 255;
        WideVectorBuilder vecBuilder(vecInfo,center,color);
        
        // Work through the segments
        for (unsigned int ii=0;ii<pts.size();ii++)
        {
            // Get the points in display space
            Point2f geoA = pts[ii];
            Point3d localPa = coordSys->geographicToLocal3d(GeoCoord(geoA.x(),geoA.y()));
            Point3d pa = coordAdapter->localToDisplay(localPa);
            Point3d up = coordAdapter->normalForLocal(localPa);
            
            // Get a drawable ready
            int ptCount = 5;
            int triCount = 4;
            BasicDrawable *thisDrawable = getDrawable(ptCount,triCount);
            drawMbr.addPoint(geoA);
            
            vecBuilder.addPoint(pa,up,thisDrawable);
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
    Point3d center;
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
    for (SimpleIDSet::iterator it = drawIDs.begin();
         it != drawIDs.end(); ++it)
        changes.push_back(new OnOffChangeRequest(*it,enable));
}

void WideVectorSceneRep::clearContents(ChangeSet &changes)
{
    for (SimpleIDSet::iterator it = drawIDs.begin();
         it != drawIDs.end(); ++it)
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
    WhirlyKitWideVectorInfo *vecInfo = [[WhirlyKitWideVectorInfo alloc] init];
    [vecInfo parseDesc:desc];
    
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
    GeoCoord centerGeo = geoMbr.mid();
    Point3d centerDisp = scene->getCoordAdapter()->localToDisplay(scene->getCoordAdapter()->getCoordSystem()->geographicToLocal3d(centerGeo));
    builder.setCenter(centerDisp);
    
    for (ShapeSet::iterator it = shapes->begin(); it != shapes->end(); ++it)
    {
        VectorLinearRef lin = boost::dynamic_pointer_cast<VectorLinear>(*it);
        if (lin)
        {
            builder.addLinear(lin->pts);
        }
    }
    
    WideVectorSceneRep *sceneRep = builder.flush(changes);
    SimpleIdentity vecID = sceneRep->getId();
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
            for (SimpleIDSet::iterator dit = vecRep->drawIDs.begin();
                 dit != vecRep->drawIDs.end(); ++dit)
                changes.push_back(new OnOffChangeRequest((*dit), enable));
        }
    }
    
    pthread_mutex_unlock(&vecLock);
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
                for (SimpleIDSet::iterator it = sceneRep->drawIDs.begin();
                     it != sceneRep->drawIDs.end(); ++it)
                    changes.push_back(new FadeChangeRequest(*it, curTime, curTime+sceneRep->fade));
                
                // Spawn off the deletion for later
                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, sceneRep->fade * NSEC_PER_SEC),
                               scene->getDispatchQueue(),
                               ^{
                                   SimpleIDSet theIDs;
                                   theIDs.insert(sceneRep->getId());
                                   ChangeSet delChanges;
                                   removeVectors(theIDs, delChanges);
                                   scene->addChangeRequests(delChanges);
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
