/*
 *  MaplyActiveVectorObject.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 4/3/13.
 *  Copyright 2011-2013 mousebird consulting
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

#import <vector>
#import "MaplyActiveObject_private.h"
#import "MaplyActiveVectorObject.h"
#import "WhirlyGlobe.h"
#import "MaplyVectorObject_private.h"

using namespace WhirlyKit;

@implementation MaplyActiveLinearVectorObject
{
    bool changed;
    WhirlyKitShapeInfo *shapeInfo;
    bool closed;
    std::vector<Point3f> srcPts;
    // IDs for the drawables we're using
    SimpleIDSet drawIDs;
    NSDictionary *desc;
}

- (id)initWithDesc:(NSDictionary *)descDict
{
    self = [super init];
    if (!self)
        return nil;
    
    changed = false;
    desc = descDict;
    shapeInfo = [[WhirlyKitShapeInfo alloc] initWithShapes:nil desc:desc];
    
    return self;
}

- (void)setPoints:(MaplyCoordinate3d *)coords numPts:(int)numPts closed:(bool)isClosed
{
    closed = isClosed;
    srcPts.resize(numPts);
    for (unsigned int ii=0;ii<numPts;ii++)
    {
        MaplyCoordinate3d *coord = &coords[ii];
        srcPts[ii] = Point3f(coord->x,coord->y,coord->z);
    }
    changed = true;
}

- (void)setFromVector:(MaplyVectorObject *)vecObj
{
    changed = true;
    srcPts.clear();
    
    ShapeSet &shapes = vecObj.shapes;
    if (shapes.size() == 1)
    {
        VectorLinearRef lin = boost::dynamic_pointer_cast<VectorLinear>(*(shapes.begin()));
        if (lin)
        {
            srcPts.reserve(lin->pts.size());
            for (unsigned int ii=0;ii<lin->pts.size();ii++)
            {
                const Point2f &pt = lin->pts[ii];
                srcPts.push_back(Point3f(pt.x(),pt.y(),0.0));
            }
        }
    }
}

- (void)setGlobeEps:(float)globeEps
{
    _globeEps = globeEps;
    changed = true;
}

- (bool)hasUpdate
{
    return changed;
}


// Flush out changes to the scene
- (void)updateForFrame:(WhirlyKitRendererFrameInfo *)frameInfo
{
    if (!changed)
        return;
    
    std::vector<ChangeRequest *> changes;
    // Get rid of the old drawables
    for (SimpleIDSet::iterator it = drawIDs.begin();
         it != drawIDs.end(); ++it)
        changes.push_back(new RemDrawableReq(*it));
    drawIDs.clear();

    // Construct new ones
    if (srcPts.size() > 0)
    {
        float drawOffset = shapeInfo.drawOffset;
        std::vector<Point3f> outPts;
        VectorRing inPts;
        inPts.resize(srcPts.size());
        for (unsigned int ii=0;ii<srcPts.size();ii++)
        {
            Point3f &coord = srcPts[ii];
            inPts[ii] = Point2f(coord.x(),coord.y());
        }
        if (_globeEps > 0.0)
            // Note: Z buffer res at the end is a hack
            SubdivideEdgesToSurfaceGC(inPts, outPts, false, scene->getCoordAdapter(), _globeEps, drawOffset*0.0001);
        else {
            // Just use a really large number to at least run it through the reprojection
            SubdivideEdgesToSurfaceGC(inPts, outPts, false, scene->getCoordAdapter(), _globeEps, 1.0);
        }
            
        Mbr drawMbr;
        drawMbr.addPoints(inPts);
        
        ShapeDrawableBuilder drawBuild(scene->getCoordAdapter(),shapeInfo,true);
        drawBuild.addPoints(outPts, [shapeInfo.color asRGBAColor], drawMbr, shapeInfo.lineWidth, false);
        drawBuild.flush();
        drawBuild.getChanges(changes, drawIDs);
    }
    
    scene->addChangeRequests(changes);
    
    changed = false;
}

- (void)shutdown
{
    std::vector<ChangeRequest *> changes;
    // Get rid of the old drawables
    for (SimpleIDSet::iterator it = drawIDs.begin();
         it != drawIDs.end(); ++it)
        changes.push_back(new RemDrawableReq(*it));
    scene->addChangeRequests(changes);
    drawIDs.clear();
}

@end
