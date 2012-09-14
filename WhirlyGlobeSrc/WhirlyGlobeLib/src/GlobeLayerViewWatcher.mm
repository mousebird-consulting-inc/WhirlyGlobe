/*
 *  GlobeLayerViewWatcher.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/17/12.
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

#import "GlobeLayerViewWatcher.h"
#import "LayerThread.h"

using namespace WhirlyKit;

@implementation WhirlyGlobeViewState

- (id)initWithView:(WhirlyGlobeView *)globeView
{
    self = [super initWithView:globeView];
    if (self)
    {
        heightAboveGlobe = globeView.heightAboveGlobe;
        rotQuat = [globeView rotQuat];
    }
    
    return self;
}

- (void)dealloc
{
    
}

- (Vector3f)currentUp
{
	Eigen::Matrix4f modelMat = modelMatrix.inverse();
	
	Vector4f newUp = modelMat * Vector4f(0,0,1,0);
	return Vector3f(newUp.x(),newUp.y(),newUp.z());
}

- (Eigen::Vector3f)eyePos
{
	Eigen::Matrix4f modelMat = modelMatrix.inverse();
	
	Vector4f newUp = modelMat * Vector4f(0,0,1,1);
	return Vector3f(newUp.x(),newUp.y(),newUp.z());    
}

@end


@implementation WhirlyGlobeLayerViewWatcher

- (id)initWithView:(WhirlyGlobeView *)inView thread:(WhirlyKitLayerThread *)inLayerThread
{
    self = [super initWithView:inView thread:inLayerThread];
    if (self)
    {
        inView.watchDelegate = self;
        viewStateClass = [WhirlyGlobeViewState class];
    }
    
    return self;
}

@end
