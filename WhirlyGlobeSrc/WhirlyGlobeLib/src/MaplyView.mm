/*
 *  MaplyView.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/9/12.
 *  Copyright 2011-2012 mousebird consulting
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

#import "MaplyView.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation MaplyView

@synthesize loc;
@synthesize delegate;

- (id)initWithCoordAdapater:(WhirlyKit::CoordSystemDisplayAdapter *)inCoordAdapter
{
    if (self = [super init])
    {
        coordAdapter = inCoordAdapter;
        loc = Point3d(0,0,4);
        farPlane = 10.0;
    }
    
    return self;
}


- (void)cancelAnimation
{
    self.delegate = nil;
}

- (void)animate
{
    if (delegate)
        [delegate updateView:self];
}

- (float)calcZbufferRes
{
    // Note: Not right
    double delta = 0.0001;
    
    return delta;
}

- (Eigen::Matrix4d)calcModelMatrix
{
    Eigen::Affine3d trans(Eigen::Translation3d(-loc.x(),-loc.y(),-loc.z()));
    
    return trans.matrix();
}

- (double)heightAboveSurface
{
    return loc.z();
}

- (double)minHeightAboveSurface
{
    return nearPlane;
}

- (double)maxHeightAboveSurface
{
    return farPlane;
}

- (void)setLoc:(WhirlyKit::Point3d)newLoc
{
    loc = newLoc;
    [self runViewUpdates];
}

- (bool)pointOnPlaneFromScreen:(CGPoint)pt transform:(const Eigen::Matrix4d *)transform frameSize:(const Point2f &)frameSize hit:(Point3d *)hit clip:(bool)clip
{
    // Back Project the screen point into model space
    Point3d screenPt = [self pointUnproject:Point2f(pt.x,pt.y) width:frameSize.x() height:frameSize.y() clip:clip];
    
    // Run the screen point and the eye point (origin) back through
    //  the model matrix to get a direction and origin in model space
    Eigen::Matrix4d modelTrans = *transform;
    Matrix4d invModelMat = modelTrans.inverse();
    Point3d eyePt(0,0,0);
    Vector4d modelEye = invModelMat * Vector4d(eyePt.x(),eyePt.y(),eyePt.z(),1.0);
    Vector4d modelScreenPt = invModelMat * Vector4d(screenPt.x(),screenPt.y(),screenPt.z(),1.0);
    
    // Now intersect with the plane at (0,0)
    // Okay, this is kind of overkill
    Vector4d dir4 = modelScreenPt - modelEye;
    Vector3d dir(dir4.x(),dir4.y(),dir4.z());
    
    if (dir.z() == 0.0)
        return false;
    dir.normalize();
    double t = - modelEye.z() / dir.z();
    *hit = Vector3d(modelEye.x(),modelEye.y(),modelEye.z()) + dir * t;
    
    return true;
}

- (CGPoint)pointOnScreenFromPlane:(const Point3d &)inWorldLoc transform:(const Eigen::Matrix4d *)transform frameSize:(const Point2f &)frameSize
{
    Point3d worldLoc(inWorldLoc.x(),inWorldLoc.y(),inWorldLoc.z());
    
    // Run the model point through the model transform (presumably what they passed in)
    Eigen::Matrix4d modelTrans = *transform;
    Matrix4d modelMat = modelTrans;
    Vector4d screenPt = modelMat * Vector4d(worldLoc.x(),worldLoc.y(),worldLoc.z(),1.0);
    screenPt.x() /= screenPt.w();  screenPt.y() /= screenPt.w();  screenPt.z() /= screenPt.w();
    
    // Intersection with near gives us the same plane as the screen 
    Point3d ray;
    ray.x() = screenPt.x() / screenPt.w();  ray.y() = screenPt.y() / screenPt.w();  ray.z() = screenPt.z() / screenPt.w();
    ray *= -nearPlane/ray.z();
    
    // Now we need to scale that to the frame
    Point2d ll,ur;
    double near,far;
    [self calcFrustumWidth:frameSize.x() height:frameSize.y() ll:ll ur:ur near:near far:far];
    double u = (ray.x() - ll.x()) / (ur.x() - ll.x());
    double v = (ray.y() - ll.y()) / (ur.y() - ll.y());
    v = 1.0 - v;
    
    CGPoint retPt;
    retPt.x = u * frameSize.x();
    retPt.y = v * frameSize.y();
    
    return retPt;    
}

@end
