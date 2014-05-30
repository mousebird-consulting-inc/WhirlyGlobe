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
    NSString *widenType = desc[@"widentype"];
    if ([widenType isEqualToString:@"real"])
        _widenType = WideVectorReal;
    else
        _widenType = WideVectorScreen;
    _texID = [desc intForKey:@"texture" default:EmptyIdentity];
    _repeatSize = [desc floatForKey:@"repeatSize" default:6371000.0 / 20];
}

@end

namespace WhirlyKit
{
    
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
          
            if (vecInfo.widenType == WideVectorReal)
            {
                drawable = new BasicDrawable("WideVector");
            } else {
                drawable = new WideVectorDrawable();
                drawable->setProgram(vecInfo.shader);
            }
            drawMbr.reset();
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
        // Work through the segments
        double texOrg = 0.0;
        for (unsigned int ii=0;ii<pts.size()-1;ii++)
        {
            // Get the points in display space
            Point2f geoA = pts[ii];
            Point2f geoB = pts[ii+1];
            Point3d localPa = coordSys->geographicToLocal3d(GeoCoord(geoA.x(),geoA.y()));
            Point3d localPb = coordSys->geographicToLocal3d(GeoCoord(geoB.x(),geoB.y()));
            Point3d pa = coordAdapter->localToDisplay(localPa);
            Point3d pb = coordAdapter->localToDisplay(localPb);
            
            // We need the normal (with respect to the line), and its inverse
            // These are half, for half the width
            Point3d up = coordAdapter->normalForLocal(localPa);
            Point3d norm = (pb-pa).cross(up);
            norm.normalize();
            norm /= 2.0;
            Point3d revNorm = norm * -1.0;
            
            // Get a drawable ready
            int ptCount = 4;
            int triCount = 2;
            BasicDrawable *thisDrawable = getDrawable(ptCount,triCount);
            RGBAColor color = [vecInfo.color asRGBAColor];
            
            double texLen = (pa-pb).norm();
            
            int startVec = thisDrawable->getNumPoints();
            if (vecInfo.widenType == WideVectorReal)
            {
                texLen *= vecInfo.repeatSize;
                
                Point3f up3f(up.x(),up.y(),up.z());

                drawMbr.addPoint(geoA);
                drawMbr.addPoint(geoB);

                // Calculate points for the expanded linear
                Point3d corners[4];
                corners[0] = pa + revNorm * vecInfo.width - center;
                corners[1] = pa + norm * vecInfo.width - center;
                corners[2] = pb + norm * vecInfo.width - center;
                corners[3] = pb + revNorm * vecInfo.width - center;
                Point3f corners3f[4];
                for (unsigned int jj=0;jj<4;jj++)
                    corners3f[jj] = Point3f(corners[jj].x(),corners[jj].y(),corners[jj].z());
                
                thisDrawable->addPoint(corners3f[0]);
                if (vecInfo.texID != EmptyIdentity)
                    thisDrawable->addTexCoord(0, TexCoord(0.0,texOrg));
                thisDrawable->addNormal(up3f);
                thisDrawable->addColor(color);
                
                thisDrawable->addPoint(corners3f[1]);
                if (vecInfo.texID != EmptyIdentity)
                    thisDrawable->addTexCoord(0, TexCoord(1.0,texOrg));
                thisDrawable->addNormal(up3f);
                thisDrawable->addColor(color);
                
                thisDrawable->addPoint(corners3f[2]);
                if (vecInfo.texID != EmptyIdentity)
                    thisDrawable->addTexCoord(0, TexCoord(1.0,texOrg+texLen));
                thisDrawable->addNormal(up3f);
                thisDrawable->addColor(color);
                
                thisDrawable->addPoint(corners3f[3]);
                if (vecInfo.texID != EmptyIdentity)
                    thisDrawable->addTexCoord(0, TexCoord(0.0,texOrg+texLen));
                thisDrawable->addNormal(up3f);
                thisDrawable->addColor(color);
                
                thisDrawable->addTriangle(BasicDrawable::Triangle(startVec+0,startVec+1,startVec+3));
                thisDrawable->addTriangle(BasicDrawable::Triangle(startVec+1,startVec+2,startVec+3));
            } else {
                texLen *= vecInfo.repeatSize;

                WideVectorDrawable *wideDrawable = (WideVectorDrawable *)thisDrawable;
                drawMbr.addPoint(geoA);
                drawMbr.addPoint(geoB);
                
                // Add the "expanded" linear
                Point3f pa3f(pa.x()-center.x(),pa.y()-center.y(),pa.z()-center.z()),pb3f(pb.x()-center.x(),pb.y()-center.y(),pb.z()-center.z());
                Point3f dirR(norm.x(),norm.y(),norm.z()),dirL(revNorm.x(),revNorm.y(),revNorm.z());
                Point3f up3f(up.x(),up.y(),up.z());
                
                wideDrawable->addPoint(pa3f);
                if (vecInfo.texID != EmptyIdentity)
                    wideDrawable->addTexCoord(0, TexCoord(0.0,texOrg));
                wideDrawable->addNormal(up3f);
                wideDrawable->addDir(dirL);
                thisDrawable->addColor(color);

                wideDrawable->addPoint(pa3f);
                if (vecInfo.texID != EmptyIdentity)
                    wideDrawable->addTexCoord(0, TexCoord(1.0,texOrg));
                wideDrawable->addNormal(up3f);
                wideDrawable->addDir(dirR);
                thisDrawable->addColor(color);

                wideDrawable->addPoint(pb3f);
                if (vecInfo.texID != EmptyIdentity)
                    wideDrawable->addTexCoord(0, TexCoord(1.0,texOrg+texLen));
                wideDrawable->addNormal(up3f);
                wideDrawable->addDir(dirR);
                thisDrawable->addColor(color);

                wideDrawable->addPoint(pb3f);
                if (vecInfo.texID != EmptyIdentity)
                    wideDrawable->addTexCoord(0, TexCoord(0.0,texOrg+texLen));
                wideDrawable->addNormal(up3f);
                wideDrawable->addDir(dirL);
                thisDrawable->addColor(color);

                thisDrawable->addTriangle(BasicDrawable::Triangle(startVec+0,startVec+1,startVec+3));
                thisDrawable->addTriangle(BasicDrawable::Triangle(startVec+1,startVec+2,startVec+3));
            }
            
            texOrg += texLen;
        }
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
