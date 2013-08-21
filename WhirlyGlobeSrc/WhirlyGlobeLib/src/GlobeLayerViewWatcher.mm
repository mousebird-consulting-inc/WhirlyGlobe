/*
 *  GlobeLayerViewWatcher.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/17/12.
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

#import "GlobeLayerViewWatcher.h"
#import "LayerThread.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation WhirlyGlobeViewState

- (id)initWithView:(WhirlyGlobeView *)globeView renderer:(WhirlyKitSceneRendererES *)renderer
{
    self = [super initWithView:globeView renderer:renderer];
    if (self)
    {
        _heightAboveGlobe = globeView.heightAboveGlobe;
        _rotQuat = [globeView rotQuat];
    }
    
    return self;
}

- (void)dealloc
{
    
}

- (Vector3d)currentUp
{
	Eigen::Matrix4d modelMat = self.modelMatrix.inverse();
	
	Vector4d newUp = modelMat * Vector4d(0,0,1,0);
	return Vector3d(newUp.x(),newUp.y(),newUp.z());
}


- (bool)pointOnSphereFromScreen:(CGPoint)pt transform:(const Eigen::Matrix4d *)transform frameSize:(const Point2f &)frameSize hit:(Point3d *)hit
{
	// Back project the point from screen space into model space
	Point3d screenPt = [self pointUnproject:Point2d(pt.x,pt.y) width:frameSize.x() height:frameSize.y() clip:true];
	
	// Run the screen point and the eye point (origin) back through
	//  the model matrix to get a direction and origin in model space
	Eigen::Matrix4d modelTrans = *transform;
	Matrix4d invModelMat = modelTrans.inverse();
	Point3d eyePt(0,0,0);
	Vector4d modelEye = invModelMat * Vector4d(eyePt.x(),eyePt.y(),eyePt.z(),1.0);
	Vector4d modelScreenPt = invModelMat * Vector4d(screenPt.x(),screenPt.y(),screenPt.z(),1.0);
	
	// Now intersect that with a unit sphere to see where we hit
	Vector4d dir4 = modelScreenPt - modelEye;
	Vector3d dir(dir4.x(),dir4.y(),dir4.z());
	if (IntersectUnitSphere(Vector3d(modelEye.x(),modelEye.y(),modelEye.z()), dir, *hit))
		return true;
	
	// We need the closest pass, if that didn't work out
	Vector3d orgDir(-modelEye.x(),-modelEye.y(),-modelEye.z());
	orgDir.normalize();
	dir.normalize();
	Vector3d tmpDir = orgDir.cross(dir);
	Vector3d resVec = dir.cross(tmpDir);
	*hit = -resVec.normalized();
	
	return false;
}

@end


@implementation WhirlyGlobeLayerViewWatcher

- (id)initWithView:(WhirlyGlobeView *)inView thread:(WhirlyKitLayerThread *)inLayerThread
{
    self = [super initWithView:inView thread:inLayerThread];
    if (self)
    {
        [inView addWatcherDelegate:self];
        super.viewStateClass = [WhirlyGlobeViewState class];
    }
    
    return self;
}

@end
