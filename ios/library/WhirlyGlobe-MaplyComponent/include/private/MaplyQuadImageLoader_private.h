/*
 *  MaplyQuadImageLoader_private.h
 *
 *  Created by Steve Gifford on 4/10/18.
 *  Copyright 2012-2018 Saildrone Inc
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

#import "loading/MaplyQuadLoader.h"
#import "QuadTileBuilder.h"
#import "MaplyQuadSampler_private.h"
#import "MaplyQuadLoader_private.h"
#import "loading/MaplyQuadImageLoader.h"
#import "QuadImageFrameLoader_iOS.h"

@interface MaplyQuadImageLoaderBase()

- (instancetype)initWithViewC:(NSObject<MaplyRenderControllerProtocol> *)inViewC;

@end
