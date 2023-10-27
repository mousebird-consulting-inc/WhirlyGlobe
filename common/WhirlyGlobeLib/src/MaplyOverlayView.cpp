/*  MaplyOverlayView.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Tim Sylvester on 8/22/2023
 *  Copyright 2023-2023 mousebird consulting
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

#import "Platform.h"
#import "MaplyOverlayView.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace Maply
{

MapOverlayView::MapOverlayView(WhirlyKit::CoordSystemDisplayAdapter *inCoordAdapter) :
    MapView(inCoordAdapter),
    mvp(Eigen::Matrix4d::Identity())
{
}

MapOverlayView::MapOverlayView(const MapOverlayView &that) :
    MapView(that), mvp(that.mvp)
{
}

Eigen::Matrix4d MapOverlayView::calcModelMatrix() const
{
    return mvp;
}

Eigen::Matrix4d MapOverlayView::calcViewMatrix() const
{
    return Eigen::Matrix4d::Identity();
}

Eigen::Matrix4d MapOverlayView::calcProjectionMatrix(WhirlyKit::Point2f frameBufferSize,float margin) const
{
    return Eigen::Matrix4d::Identity();
}

void MapOverlayView::getOffsetMatrices(Matrix4dVector &offsetMatrices, const WhirlyKit::Point2f &, float) const
{
    offsetMatrices.push_back(Matrix4d::Identity());
}

void MapOverlayView::assignMatrix(const Eigen::Matrix4d &mat)
{
    mvp = mat;
}

void MapOverlayView::setLoc(const WhirlyKit::Point3d &newLoc, bool runUpdates)
{
}

void MapOverlayView::setRotAngle(double newRotAngle, bool runUpdates)
{
}

}
