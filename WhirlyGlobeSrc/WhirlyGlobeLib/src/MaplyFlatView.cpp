/*
 *  MaplyFlatView.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/2/13.
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

#import "MaplyFlatView.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace Maply
{
    
FlatView::FlatView(WhirlyKit::CoordSystemDisplayAdapter *coordAdapter)
: MapView(coordAdapter)
{
    nearPlane = 1;
    farPlane = -1;
    ll = Point2d(-1,-1);
    ur = Point2d(1,1);
}

Eigen::Matrix4d FlatView::calcModelMatrix()
{
    return Eigen::Matrix4d::Identity();
}

Eigen::Matrix4d FlatView::calcProjectionMatrix(Point2f frameBufferSize,float margin)
{
    // If the framebuffer isn't set up, just return something simple
    if (frameBufferSize.x() == 0.0 || frameBufferSize.y() == 0.0)
    {
        Eigen::Matrix4d projMat;
        projMat.setIdentity();
        return projMat;
    }
    
    double left,right,top,bot,near,far;
    left = ll.x();
    right = ur.x();
    top = ur.y();
    bot = ll.y();
    near = nearPlane;
    far = farPlane;
    
    // Borrowed from the "OpenGL ES 2.0 Programming" book
    // Orthogonal matrix
    Point3d delta(right-left,top-bot,far-near);
    Eigen::Matrix4d projMat;
    projMat.setIdentity();
    projMat(0,0) = 2.0 / delta.x();
    projMat(0,3) = -(right + left) / delta.x();
    projMat(1,1) = 2.0 / delta.y();
    projMat(1,3) = -(top + bot) / delta.y();
    projMat(2,2) = -2.0 / delta.z();
    projMat(2,3) = - (near + far) / delta.z();
    
    return projMat;
}

double FlatView::heightAboveSurface()
{
    return 0.0;
}

double FlatView::minHeightAboveSurface()
{
    return 0.0;
}

double FlatView::maxHeightAboveSurface()
{
    return 0.0;
}
    
Point2d FlatView::screenSizeInDisplayCoords(Point2f &frameSize)
{
    Point2d screenSize(0,0);
    if (frameSize.x() == 0.0 || frameSize.y() == 0.0)
        return screenSize;
    
    screenSize = ur-ll;
    
    return screenSize;
}

void FlatView::setWindow(const WhirlyKit::Point2d &inLL,const WhirlyKit::Point2d &inUR)
{
    Point3d scale = coordAdapter->getScale();
    
    ll = Point2d(inLL.x()*scale.x(),inLL.y()*scale.y());
    ur = Point2d(inUR.x()*scale.x(),inUR.y()*scale.y());

    runViewUpdates();
}

}
