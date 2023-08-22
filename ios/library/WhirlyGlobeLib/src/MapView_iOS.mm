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

void MapViewOverlay_iOS::getOffsetMatrices(Matrix4dVector &offsetMatrices,const WhirlyKit::Point2f &frameBufferSize,float bufferSizeX) const
{
    offsetMatrices.push_back(Matrix4d::Identity());
}

void MapViewOverlay_iOS::setLoc(WhirlyKit::Point3d newLoc)
{
    return;
}

void MapViewOverlay_iOS::setLoc(const WhirlyKit::Point3d &newLoc,bool runUpdates)
{
    return;
}

void MapViewOverlay_iOS::setRotAngle(double newRotAngle,bool runUpdates)
{
    return;
}

}
