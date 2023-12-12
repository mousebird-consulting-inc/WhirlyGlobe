/*
*  MaplyStandaloneRenderController.h
*  WhirlyGlobeComponent
*
*  Created by Steve Gifford on 10/19/23.
*  Copyright 2011-2023 mousebird consulting
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
#import <WhirlyGlobe/MaplyRenderController.h>
#import <WhirlyGlobe/MaplyRemoteTileFetcher.h>
#import <Metal/Metal.h>

@interface MaplyStandaloneRenderController : MaplyRenderController

/// Initialize as an offline renderer of a given target size with default renderer (Metal)
- (instancetype __nullable)initWithSize:(CGSize)size separateAlpha:(bool)separateAlpha;

/// If we're rerendering the whole frame we don't need a clear
/// The clear seems to also cause problems in some modes
- (void)setClearToLoad:(bool)clearToLoad;

/// Ask the renderer to render to this texture assumine we're called every period (for animation)
- (BOOL)renderTo:(id<MTLTexture> __nonnull)colorTexture alphaTex:(id<MTLTexture> __nullable)alphaTexture period:(NSTimeInterval)howLong;

@end

