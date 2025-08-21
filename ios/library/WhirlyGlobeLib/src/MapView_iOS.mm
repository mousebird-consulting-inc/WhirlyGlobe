/*
 *  MapView_iOS.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/30/19.
 *  Copyright 2011-2022 mousebird consulting
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

#import <UIKit/UIKit.h>
#import "MapView_iOS.h"
#import "GlobeView_iOS.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace Maply {
    
MapView_iOS::MapView_iOS(WhirlyKit::CoordSystemDisplayAdapter *coordAdapter)
    : MapView(coordAdapter)
{
    tag = [[NSObject alloc] init];
}
    
void MapView_iOS::setDelegate(MapViewAnimationDelegateRef delegate)
{
    MapView::setDelegate(delegate);
    
    if (!delegate)
        [[NSNotificationCenter defaultCenter] postNotificationName:kWKViewAnimationEnded object:tag];
    else {
        [[NSNotificationCenter defaultCenter] postNotificationName:kWKViewAnimationStarted object:tag];
    }
}

void MapView_iOS::cancelAnimation()
{
    bool hadDelegate = delegate != nil;

    MapView::cancelAnimation();
    
    if (hadDelegate)
        [[NSNotificationCenter defaultCenter] postNotificationName:kWKViewAnimationEnded object:tag];
}

MapViewOverlay_iOS::MapViewOverlay_iOS(WhirlyKit::CoordSystemDisplayAdapter *coordAdapter)
    : MapView_iOS(coordAdapter)
{
    tag = [[NSObject alloc] init];
}
    
void MapViewOverlay_iOS::setDelegate(MapViewAnimationDelegateRef delegate)
{
    MapView::setDelegate(delegate);
    
    if (!delegate)
        [[NSNotificationCenter defaultCenter] postNotificationName:kWKViewAnimationEnded object:tag];
    else {
        [[NSNotificationCenter defaultCenter] postNotificationName:kWKViewAnimationStarted object:tag];
    }
}

void MapViewOverlay_iOS::cancelAnimation()
{
    bool hadDelegate = delegate != nil;

    MapView::cancelAnimation();
    
    if (hadDelegate)
        [[NSNotificationCenter defaultCenter] postNotificationName:kWKViewAnimationEnded object:tag];
}

Eigen::Matrix4d MapViewOverlay_iOS::calcModelMatrix() const
{
    return mvp;
}

Eigen::Matrix4d MapViewOverlay_iOS::calcViewMatrix() const
{
    return Eigen::Matrix4d::Identity();
}

Eigen::Matrix4d MapViewOverlay_iOS::calcProjectionMatrix(Point2f frameBufferSize,float margin) const
{
    return Eigen::Matrix4d::Identity();
}

void MapViewOverlay_iOS::assignMatrix(const Eigen::Matrix4d &mat)
{
    mvp = mat;
}

void MapViewOverlay_iOS::assignWorldSize(double inWorldSize)
{
    worldSize = inWorldSize;
}


void MapViewOverlay_iOS::getOffsetMatrices(Matrix4dVector &offsetMatrices,const WhirlyKit::Point2f &frameBufferSize,float bufferSizeX) const
{
    const Point3d scale = coordAdapter->getScale();
    
    Point3f ll,ur;
    if (wrap && coordAdapter && coordAdapter->getBounds(ll, ur))
    {
        // Figure out where we are, first off
        const GeoCoord geoLL = coordAdapter->getCoordSystem()->localToGeographic(ll);
        const GeoCoord geoUR = coordAdapter->getCoordSystem()->localToGeographic(ur);
        const float spanX = geoUR.x()-geoLL.x();
        const float offX = loc.x()*scale.x()-geoLL.x();
        const auto num = (int)floor(offX/spanX);
        const float localSpanX = ur.x()-ll.x();
//        const float localSpanX = worldSize;
        const auto llPost = mvp * Vector4d(ll.x(),ll.y(),ll.z(),1.0);
        const auto urPost = mvp * Vector4d(ur.x(),ur.y(),ur.z(),1.0);
        const float postSpanX = abs(urPost.x() - llPost.x());

        // See if the framebuffer lands in any of the potential matrices
        const Matrix4d testMat = mvp;
        const MbrD screenMbr({ -1.0, -1.0 }, { 1.0, 1.0 });

        for (int thisNum : { num, num - 1, num + 1 })
        {
            const Affine3d offsetMat(Translation3d(thisNum*postSpanX,0.0,0.0));
            const Point3d testPts[4] = {
                {  thisNum      * localSpanX + ll.x() - bufferSizeX, ll.y(), 0.0 },
                { (thisNum + 1) * localSpanX + ll.x() + bufferSizeX, ll.y(), 0.0 },
                { (thisNum + 1) * localSpanX + ll.x() + bufferSizeX, ur.y(), 0.0 },
                {  thisNum      * localSpanX + ll.x() - bufferSizeX, ur.y(), 0.0 },
            };
            MbrD testMbr;
            for (unsigned int jj=0;jj<4;jj++)
            {
                testMbr.addPoint(Slice(Clip(Point4d(testMat * Pad(testPts[jj], 1.0)))));
            }
            if (testMbr.overlaps(screenMbr))
            {
                offsetMatrices.push_back(offsetMat.matrix());
            }
        }
    }

    if (offsetMatrices.empty())
    {
        offsetMatrices.push_back(Matrix4d::Identity());
    }
}

void MapViewOverlay_iOS::setLoc(WhirlyKit::Point3d newLoc)
{
    return;
}

void MapViewOverlay_iOS::setLoc(const WhirlyKit::Point3d &newLoc,bool runUpdates)
{
    loc = newLoc;
    
    if (runUpdates)
        runViewUpdates();
}

void MapViewOverlay_iOS::setRotAngle(double newRotAngle,bool runUpdates)
{
    rotAngle = newRotAngle;
    
    if (runUpdates)
        runViewUpdates();
}

void MapViewOverlay_iOS::assignScreenSizeInDisplayCoords(double size)
{
    overrideSize = size;
}

Point2d MapViewOverlay_iOS::screenSizeInDisplayCoords(const Point2f &frameSize)
{
    if (overrideSize != 0.0) {
        return Point2d(overrideSize,overrideSize);
    }
    
    Point2d screenSize(0,0);
    if (frameSize.x() == 0.0 || frameSize.y() == 0.0)
        return screenSize;
    
    // We need to run the frustrum back through the MVP to get real world coordinates(ish)
    auto mvpInv = mvp.inverse();
    
    Point4d ll = mvpInv * Vector4d(-1.0,-1.0,0.0,1.0);
    Point4d ur = mvpInv * Vector4d(1.0,1.0,0.0,1.0);
    ll = ll/ll.w();
    ur = ur/ur.w();

    screenSize = 0.5 * (Point2d(ur.x(),ur.y()) - Point2d(ll.x(),ll.y()));
            
    return screenSize;
}

}
