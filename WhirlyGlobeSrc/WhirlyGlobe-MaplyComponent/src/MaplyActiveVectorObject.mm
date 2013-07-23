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
#import "MaplySharedAttributes.h"

using namespace WhirlyKit;

// Sample a great circle and throw in an interpolated height at each point
// Note: Borrowed from the interaction layer
static void SampleGreatCircle(const VectorRing &inPts,std::vector<Point3f> &pts,float height,float eps,int minPts,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter)
{
    bool isFlat = coordAdapter->isFlat();
    
    // We can subdivide the great circle with this routine
    if (isFlat)
    {
        pts.reserve(inPts.size());
        for (unsigned int ii=0;ii<inPts.size();ii++)
        {
            const Point2f &pt = inPts[ii];
            pts.push_back(coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(pt.x(),pt.y()))));
        }
    } else {
        SubdivideEdgesToSurfaceGC(inPts, pts, false, coordAdapter, eps, 0.0, minPts);
        
        // To apply the height, we'll need the total length
        float totLen = 0;
        for (int ii=0;ii<pts.size()-1;ii++)
        {
            float len = (pts[ii+1]-pts[ii]).norm();
            totLen += len;
        }
        
        // Note: Hack!
        if (totLen < 0.009)
            height = height / 10;
        if (totLen < 0.0009)
            height = height / 10;
        
        // Now we'll walk along, apply the height (max at the middle)
        float lenSoFar = 0.0;
        for (unsigned int ii=0;ii<pts.size();ii++)
        {
            Point3f &pt = pts[ii];
            float len = (pts[ii+1]-pt).norm();
            float t = lenSoFar/totLen;
            lenSoFar += len;
            
            // Parabolic curve
            float b = 4*height;
            float a = -b;
            float thisHeight = a*(t*t) + b*t;
            
            if (isFlat)
                pt.z() = thisHeight;
            else
                pt *= 1.0+thisHeight;
        }
    }
}

@implementation MaplyActiveLinearVectorObject
{
    bool changed;
    WhirlyKitShapeInfo *shapeInfo;
    float height;
    int minSample;
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
    height = [desc floatForKey:kMaplyVecHeight default:0.0];
    minSample = [desc intForKey:kMaplyVecMinSample default:1];
    
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
    
    ChangeSet changes;
    // Get rid of the old drawables
    for (SimpleIDSet::iterator it = drawIDs.begin();
         it != drawIDs.end(); ++it)
        changes.push_back(new RemDrawableReq(*it));
    drawIDs.clear();

    // Construct new ones
    if (srcPts.size() > 0)
    {
        std::vector<Point3f> outPts;
        VectorRing inPts;
        inPts.resize(srcPts.size());
        for (unsigned int ii=0;ii<srcPts.size();ii++)
        {
            Point3f &coord = srcPts[ii];
            inPts[ii] = Point2f(coord.x(),coord.y());
        }
        if (_globeEps > 0.0)
        {
            SampleGreatCircle(inPts, outPts, height, _globeEps, minSample, scene->getCoordAdapter());
            // Note: Z buffer res at the end is a hack
//            SubdivideEdgesToSurfaceGC(inPts, outPts, false, scene->getCoordAdapter(), _globeEps, drawOffset*0.0001);
        } else {
            // Just use a really large number to at least run it through the reprojection
            SubdivideEdgesToSurfaceGC(inPts, outPts, false, scene->getCoordAdapter(), _globeEps, 1.0, minSample);
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
    ChangeSet changes;
    // Get rid of the old drawables
    for (SimpleIDSet::iterator it = drawIDs.begin();
         it != drawIDs.end(); ++it)
        changes.push_back(new RemDrawableReq(*it));
    scene->addChangeRequests(changes);
    drawIDs.clear();
}

@end
