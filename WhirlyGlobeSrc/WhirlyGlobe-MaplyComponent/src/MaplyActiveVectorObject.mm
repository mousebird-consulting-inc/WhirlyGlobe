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
#import "ShapeManager.h"

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
    MaplyVectorObject *vecObj;
    float globeEps;
    NSDictionary *desc;
    SimpleIDSet shapeIDs;
}

- (void)setWithVector:(MaplyVectorObject *)newVecObj desc:(NSDictionary *)inDesc eps:(float)inEps
{
    if (dispatchQueue)
    {
        dispatch_async(dispatchQueue,
                       ^{
                           @synchronized(self)
                           {
                               vecObj = newVecObj;
                               desc = inDesc;
                               globeEps = inEps;
                               [self update];
                           }
                       }
                       );
    } else {
        @synchronized(self)
        {
            vecObj = newVecObj;
            desc = inDesc;
            globeEps = inEps;
            [self update];
        }
    }
}

- (void)extractPoints:(VectorRing &)pts fromVec:(MaplyVectorObject *)theVecObj
{
    if (theVecObj.shapes.empty())
        return;
    
    VectorArealRef areal = boost::dynamic_pointer_cast<VectorAreal>(*(theVecObj.shapes.begin()));
    if (areal && areal->loops.size())
    {
        pts = areal->loops[0];
        return;
    }
    
    VectorLinearRef lin = boost::dynamic_pointer_cast<VectorLinear>(*(theVecObj.shapes.begin()));
    if (lin)
    {
        pts.resize(lin->pts.size());
        for (unsigned int ii=0;ii<lin->pts.size();ii++)
        {
            Point2f &coord = lin->pts[ii];
            pts[ii] = Point2f(coord.x(),coord.y());
        }
    }
}

- (void)update
{
    ChangeSet changes;
    
    ShapeManager *shapeManager = (ShapeManager *)scene->getManager(kWKShapeManager);
    
    if (shapeManager && !shapeIDs.empty())
        shapeManager->removeShapes(shapeIDs, changes);
    shapeIDs.clear();
    
    // Construct new ones
    if (vecObj)
    {
        float height = [desc floatForKey:kMaplyVecHeight default:0.0];
        float minSample = [desc intForKey:kMaplyVecMinSample default:1];

        VectorRing inPts;
        [self extractPoints:inPts fromVec:vecObj];
        std::vector<Point3f> outPts;
        
        if (globeEps > 0.0)
        {
            SampleGreatCircle(inPts, outPts, height, globeEps, minSample, scene->getCoordAdapter());
            // Note: Z buffer res at the end is a hack
//            SubdivideEdgesToSurfaceGC(inPts, outPts, false, scene->getCoordAdapter(), _globeEps, drawOffset*0.0001);
        } else {
            // Just use a really large number to at least run it through the reprojection
            SubdivideEdgesToSurfaceGC(inPts, outPts, false, scene->getCoordAdapter(), globeEps, 1.0, minSample);
        }
        
        WhirlyKitShapeLinear *lin = [[WhirlyKitShapeLinear alloc] init];
        lin.pts = outPts;
        if (shapeManager)
        {
            SimpleIdentity shapeID = shapeManager->addShapes(@[lin], desc, changes);
            if (shapeID != EmptyIdentity)
                shapeIDs.insert(shapeID);
        }
    }
    
    scene->addChangeRequests(changes);
}

- (void)shutdown
{
    @synchronized(self)
    {
        vecObj = nil;
        [self update];
    }
}

@end
