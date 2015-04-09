/*
 *  MaplyZoomGestureDelegate_private.h
 *
 *
 *  Created by Jesse Crocker on 2/4/14.
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

#import <Foundation/Foundation.h>
#import "MaplyView.h"
#import "EAGLView.h"
#import "SceneRendererES.h"

using namespace WhirlyKit;

@interface MaplyZoomGestureDelegate (_private)

- (bool)withinBounds:(Point3d &)loc view:(UIView *)view renderer:(WhirlyKitSceneRendererES *)sceneRender;

@end
