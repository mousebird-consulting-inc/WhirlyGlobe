/*
 *  MaplyFlatView.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/2/13.
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

#import "MaplyFlatView.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation MaplyFlatView
{
    Point3d _loc;
}

- (id)initWithCoordAdapter:(WhirlyKit::CoordSystemDisplayAdapter *)inCoordAdapter
{
    self = [super initWithCoordAdapter:inCoordAdapter];
    if (!self)
        return nil;
    
    _loc = Point3d(0,0,0);
    super.nearPlane = 1;
    super.farPlane = -1;
    _extents = Mbr(Point2f(-M_PI,-M_PI/2.0),Point2f(M_PI,M_PI/2.0));
    _windowSize = Point2f(1.0,1.0);
    _contentOffset = Point2f(0,0);
    
    return self;
}

- (Eigen::Matrix4d)calcModelMatrix
{
//    Point2d mid((extents.ll().x()+extents.ur().x())/2.0,(extents.ll().y()+extents.ur().y())/2.0);
//    Eigen::Affine3d trans(Eigen::Translation3d(-mid.x(),-mid.y(),0.0));
//                          
    
    Eigen::Affine3d scale(Eigen::AlignedScaling3d(2.0 / (_extents.ur().x() - _extents.ll().x()),2.0 / (_extents.ur().y() - _extents.ll().y()),1.0));

    return scale.matrix();
}

- (Eigen::Matrix4d)calcProjectionMatrix:(Point2f)frameBufferSize margin:(float)margin
{
    // If the framebuffer isn't set up, just return something simple
    if (frameBufferSize.x() == 0.0 || frameBufferSize.y() == 0.0)
    {
        Eigen::Matrix4d projMat;
        projMat.setIdentity();
        return projMat;
    }
    
    double left,right,top,bot,near,far;
    double contentOffsetY = _windowSize.y() - frameBufferSize.y() - _contentOffset.y();
    left = 2.0 * _contentOffset.x() / (_windowSize.x()) - 1.0;
    right = 2.0 * (_contentOffset.x() + frameBufferSize.x()) / _windowSize.x() - 1.0;
    top = 2.0 * (contentOffsetY + frameBufferSize.y()) / _windowSize.y() - 1.0;
    bot = 2.0 * contentOffsetY / _windowSize.y() - 1.0;
    near = super.nearPlane;
    far = super.farPlane;
    
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

- (double)heightAboveSurface
{
    return 0.0;
}

- (double)minHeightAboveSurface
{
    return 0.0;
}

- (double)maxHeightAboveSurface
{
    return 0.0;
}

- (void)setLoc:(WhirlyKit::Point3d)newLoc
{
    _loc = newLoc;
    _loc.z() = 0.0;
}

- (void)setExtents:(WhirlyKit::Mbr)inExtents
{
    _extents = inExtents;
    
    [self runViewUpdates];
}

- (void)setWindowSize:(WhirlyKit::Point2f)inWindowSize contentOffset:(WhirlyKit::Point2f)inContentOffset
{
    _windowSize = inWindowSize;
    _contentOffset = inContentOffset;

    [self runViewUpdates];    
}

@end
