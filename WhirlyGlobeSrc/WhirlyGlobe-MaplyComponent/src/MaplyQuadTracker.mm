/*
 *  MaplyQuadTracker.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 5/27/15.
 *  Copyright 2011-2015 mousebird consulting
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

#import "MaplyQuadTracker_private.h"
#import "MaplyCoordinateSystem_private.h"
#import "WhirlyGlobeViewController_private.h"

using namespace WhirlyKit;
using namespace Eigen;

namespace WhirlyKit
{
class TileWrapper
{
public:
    TileWrapper() { }
    TileWrapper(MaplyTileID tileID) : tileID(tileID) { }
    
    bool operator < (const TileWrapper &that) const
    {
        if (tileID.level == that.tileID.level)
        {
            if (tileID.y == that.tileID.y)
                return tileID.x < that.tileID.x;
            return tileID.y < that.tileID.y;
        }
        return tileID.level < that.tileID.level;
    }
    
    MaplyTileID tileID;
};
}

@implementation MaplyQuadTracker
{
    CoordSystemDisplayAdapter *coordAdapter;
    WhirlyGlobeView *globeView;
    WhirlyKitSceneRendererES *renderer;
    std::set<TileWrapper> tiles;
}

- (id)initWithViewC:(WhirlyGlobeViewController *)viewC
{
    self = [super init];
    if (!self)
        return nil;
    
    coordAdapter = viewC->visualView.coordAdapter;
    renderer = viewC->sceneRenderer;
    globeView = viewC->globeView;
    
    return self;
}

- (void)addTile:(MaplyTileID)tileID
{
    @synchronized(self)
    {
        TileWrapper tile(tileID);
        if (tiles.find(tile) == tiles.end())
            tiles.insert(tile);
    }
}

- (void)removeTile:(MaplyTileID)tileID
{
    @synchronized(self)
    {
        TileWrapper tile(tileID);
        auto it = tiles.find(tile);
        if (it != tiles.end())
            tiles.erase(it);
    }
}

- (void)tiles:(MaplyQuadTrackerPointReturn *)tilesInfo forPoints:(int)numPts
{
    if (!_coordSys)
        return;
    Point2d frameSize(renderer.framebufferWidth,renderer.framebufferHeight);
    
    @synchronized(self)
    {
        Point3d hit;
        Eigen::Matrix4d modelTrans = [globeView calcFullMatrix];
        Matrix4d invModelMat = modelTrans.inverse();
        Point3d eyePt(0,0,0);
        Vector4d modelEye = invModelMat * Vector4d(eyePt.x(),eyePt.y(),eyePt.z(),1.0);
        MaplyBoundingBox wholeMbr;
        [_coordSys getBoundsLL:&wholeMbr.ll ur:&wholeMbr.ur];
        double mbrSpanX = wholeMbr.ur.x - wholeMbr.ll.x;
        double mbrSpanY = wholeMbr.ur.y - wholeMbr.ll.y;
        
        Point2d ll,ur;
        double near,far;
        [globeView calcFrustumWidth:frameSize.x() height:frameSize.y() ll:ll ur:ur near:near far:far];
        
        // Work through the points to test
        for (unsigned int ii=0;ii<numPts;ii++)
        {
            MaplyQuadTrackerPointReturn *trackInfo = &tilesInfo[ii];
            double u = trackInfo->screenU;
            double v = 1.0 - trackInfo->screenV;
            
            // Now come up with a point in 3 space between ll and ur
            Point3d viewPlaneLoc(u * (ur.x()-ll.x()) + ll.x(), v * (ur.y()-ll.y()) + ll.y(),-near);
            
            // Run the screen point and the eye point (origin) back through
            //  the model matrix to get a direction and origin in model space
            Vector4d modelScreenPt = invModelMat * Vector4d(viewPlaneLoc.x(),viewPlaneLoc.y(),viewPlaneLoc.z(),1.0);
            
            // Now intersect that with a unit sphere to see where we hit
            Vector4d dir4 = modelScreenPt - modelEye;
            Vector3d dir(dir4.x(),dir4.y(),dir4.z());
            Point3d hit;
            double t;
            if (IntersectUnitSphere(Vector3d(modelEye.x(),modelEye.y(),modelEye.z()), dir, hit, &t) && t > 0.0)
            {
                Point3d localPt = globeView.coordAdapter->displayToLocal(hit);
                // This point is in the local (tile) system
                Point3d coordPt = CoordSystemConvert3d(globeView.coordAdapter->getCoordSystem(), _coordSys->coordSystem, localPt);
                trackInfo->locX = coordPt.x();
                trackInfo->locY = coordPt.y();

                // Clip to the overall bounds
                trackInfo->tileU = (coordPt.x()-wholeMbr.ll.x)/mbrSpanX;
                trackInfo->tileV = (coordPt.y()-wholeMbr.ll.y)/mbrSpanY;
                trackInfo->tileU = std::min(trackInfo->tileU, 1.0);  trackInfo->tileU = std::max(trackInfo->tileU,0.0);
                trackInfo->tileV = std::min(trackInfo->tileV, 1.0);  trackInfo->tileV = std::max(trackInfo->tileV,0.0);

                // Dive down looking for the highest resolution tile available
                trackInfo->tileID.level = 0;
                trackInfo->tileID.x = 0;
                trackInfo->tileID.y = 0;
                for (;;)
                {
                    MaplyTileID nextTile;
                    nextTile.level = trackInfo->tileID.level+1;
                    int childX = (trackInfo->tileU < 0.5) ? 0 : 1;
                    int childY = (trackInfo->tileV < 0.5) ? 0 : 1;
                    nextTile.x = 2*trackInfo->tileID.x + childX;
                    nextTile.y = 2*trackInfo->tileID.y + childY;
                    
                    // See if this tile is here
                    TileWrapper testTile(nextTile);
                    if (tiles.find(testTile) == tiles.end())
                        break;
                    trackInfo->tileID = nextTile;
                    trackInfo->tileU = 2.0*trackInfo->tileU - childX*1.0;
                    trackInfo->tileV = 2.0*trackInfo->tileV - childY*1.0;
                }
            } else {
                trackInfo->tileID.level = -1;
            }
        }
    }
}

@end
