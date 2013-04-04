/*
 *  WhirlyKitView.mm
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

#import "WhirlyVector.h"
#import "WhirlyKitView.h"
#import "WhirlyGeometry.h"

using namespace WhirlyKit;
using namespace Eigen;

@implementation WhirlyKitView

@synthesize fieldOfView,imagePlaneSize,nearPlane,farPlane;
@synthesize coordAdapter;
@synthesize lastChangedTime;
@synthesize continuousZoom;

- (id)init
{
	if ((self = [super init]))
    {
        fieldOfView = 60.0 / 360.0 * 2 * (float)M_PI;  // 60 degree field of view
		nearPlane = 0.001;
		imagePlaneSize = nearPlane * tanf(fieldOfView / 2.0);
		farPlane = 4.0;
        lastChangedTime = CFAbsoluteTimeGetCurrent();
        continuousZoom = false;
    }
    
    return self;
}


- (void)calcFrustumWidth:(unsigned int)frameWidth height:(unsigned int)frameHeight ll:(Point2f &)ll ur:(Point2f &)ur near:(float &)near far:(float &)far
{
	ll.x() = -imagePlaneSize;
	ur.x() = imagePlaneSize;
	float ratio =  ((float)frameHeight / (float)frameWidth);
	ll.y() = -imagePlaneSize * ratio;
	ur.y() = imagePlaneSize * ratio ;
	near = nearPlane;
	far = farPlane;
}

- (void)cancelAnimation
{
    
}

- (void)animate
{
}

- (float)calcZbufferRes
{
    return 1.0;
}

/// Generate the model view matrix for use by OpenGL.
- (Eigen::Matrix4f)calcModelMatrix
{
    Eigen::Matrix4f ident = ident.Identity();
    return ident;
}

- (Eigen::Matrix4f)calcViewMatrix
{
    Eigen::Matrix4f ident = ident.Identity();
    return ident;
}

- (Eigen::Matrix4f)calcFullMatrix
{
    return [self calcViewMatrix] * [self calcModelMatrix];
}

- (float)heightAboveSurface
{
    return 0.0;
}

- (Point3f)pointUnproject:(Point2f)screenPt width:(unsigned int)frameWidth height:(unsigned int)frameHeight clip:(bool)clip
{
	Point2f ll,ur;
	float near,far;
	[self calcFrustumWidth:frameWidth height:frameHeight ll:ll ur:ur near:near far:far];
	
	// Calculate a parameteric value and flip the y/v
	float u = screenPt.x() / frameWidth;
    if (clip)
    {
        u = std::max(0.0f,u);	u = std::min(1.0f,u);
    }
	float v = screenPt.y() / frameHeight;
    if (clip)
    {
        v = std::max(0.0f,v);	v = std::min(1.0f,v);
    }
	v = 1.0 - v;
	
	// Now come up with a point in 3 space between ll and ur
	Point2f mid(u * (ur.x()-ll.x()) + ll.x(), v * (ur.y()-ll.y()) + ll.y());
	return Point3f(mid.x(),mid.y(),-near);
}

/// Add a watcher delegate
- (void)addWatcherDelegate:(NSObject<WhirlyKitViewWatcherDelegate> *)delegate
{
    watchDelegates.insert(delegate);
}

/// Remove the given watcher delegate
- (void)removeWatcherDelegate:(NSObject<WhirlyKitViewWatcherDelegate> *)delegate
{
    watchDelegates.erase(delegate);
}

- (void)runViewUpdates
{
    for (WhirlyKitViewWatcherDelegateSet::iterator it = watchDelegates.begin();
         it != watchDelegates.end(); ++it)
        [(*it) viewUpdated:self];    
}

@end
