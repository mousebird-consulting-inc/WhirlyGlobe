/*  MTLView.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/20/19.
 *  Copyright 2011-2022 mousebird consulting
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

#import <UIKit/UIKit.h>
#import <MetalKit/MetalKit.h>
#import "ViewWrapper.h"

#if TARGET_OS_VISION
@interface MTLView : UIView <NSCoding,CALayerDelegate>
- (nonnull instancetype)initWithFrame:(CGRect)frameRect device:(nullable id<MTLDevice>)device NS_DESIGNATED_INITIALIZER;

@property(readonly,nullable) CAMetalLayer *mtlLayer;

@property (nonatomic) MTLPixelFormat colorPixelFormat;

@property (nonatomic) MTLPixelFormat depthStencilPixelFormat;

@property (nonatomic) MTLTextureUsage depthStencilAttachmentTextureUsage API_AVAILABLE(macos(10.15), ios(13.0));

@property (nonatomic) BOOL framebufferOnly;

@property(nonatomic) NSInteger preferredFramesPerSecond;

@property (nonatomic, readonly, nullable) id <CAMetalDrawable> currentDrawable;

- (void)draw;

@property (nonatomic, readonly, nullable) MTLRenderPassDescriptor *currentRenderPassDescriptor;

@end
#endif

/** Base class for implementing a Metal rendering view.
 This is modeled off of the example.  We subclass this for our own purposes.
 */
#if TARGET_OS_VISION
@interface WhirlyKitMTLView : MTLView<WhirlyKitViewWrapper>
#else
@interface WhirlyKitMTLView : MTKView<WhirlyKitViewWrapper>
#endif

/// Default init call
- (id _Nullable )initWithDevice:(_Nonnull id<MTLDevice>)mtlDevice;

@end
