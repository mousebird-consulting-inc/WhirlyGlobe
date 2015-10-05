/*
 *  MaplyLayerViewWatcher.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/14/12.
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

#import "MaplyLayerViewWatcher.h"
#import "LayerThread.h"

using namespace WhirlyKit;
using namespace Eigen;

@implementation MaplyViewState

- (id)initWithView:(MaplyView *)mapView renderer:(WhirlyKitSceneRendererES *)renderer
{
    self = [super initWithView:mapView renderer:renderer];
    
    if (self)
    {
        _heightAboveSurface = mapView.loc.z();
    }
    
    return self;
}

- (bool)pointOnPlaneFromScreen:(CGPoint)pt transform:(const Eigen::Matrix4d *)transform frameSize:(const Point2f &)frameSize hit:(Point3d *)hit clip:(bool)clip
{
    // Back Project the screen point into model space
    Point3d screenPt = [super pointUnproject:Point2d(pt.x,pt.y) width:frameSize.x() height:frameSize.y() clip:clip];
    
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
    *hit = Point3d(modelEye.x(),modelEye.y(),modelEye.z()) + dir * t;
    
    return true;
}

@end


@implementation MaplyLayerViewWatcher

- (id)initWithView:(MaplyView *)inView thread:(WhirlyKitLayerThread *)inLayerThread;
{
    self = [super initWithView:inView thread:inLayerThread];
    if (self)
    {
        [inView addWatcherDelegate:self];
        super.viewStateClass = [MaplyViewState class];
    }
    
    return self;
}

@end
