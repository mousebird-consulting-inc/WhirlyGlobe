/*
 *  MTLView.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/20/19.
 *  Copyright 2011-2019 mousebird consulting
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
#import <MetalKit/MetalKit.h>

#import "ViewWrapper.h"
#import "SceneRendererMTL.h"

/** OpenGL View is a
 base class for implementing an open GL rendering view.
 This is modeled off of the example.  We subclass this for
 our own purposes.
 */
@interface WhirlyKitMTLView : MTKView<WhirlyKitViewWrapper>

/// Default init call
- (id)initWithDevice:(id<MTLDevice>)mtlDevice;

/// We're only expecting this to be set once
@property (nonatomic) WhirlyKit::SceneRenderer *renderer;

@end
