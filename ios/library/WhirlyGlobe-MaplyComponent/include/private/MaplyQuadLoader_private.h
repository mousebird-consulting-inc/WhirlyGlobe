/*
 *  MaplyQuadImageLoader_private.h
 *
 *  Created by Steve Gifford on 2/12/19.
 *  Copyright 2012-2019 mousebird consulting inc
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

#import "MaplyQuadLoader.h"
#import "QuadTileBuilder.h"
#import "MaplyQuadSampler_private.h"
#import "MaplyQuadLoader_private.h"
#import "QuadDisplayLayerNew.h"
#import "QuadLoaderReturn.h"

@interface MaplyQuadLoaderBase()
{
@public
    NSObject<MaplyTileFetcher> *tileFetcher;
    NSObject<MaplyLoaderInterpreter> *loadInterp;
    
    int minLevel,maxLevel;
    
    MaplyQuadSamplingLayer *samplingLayer;
}

- (instancetype)initWithViewC:(MaplyBaseViewController *)inViewC;

@end

@interface MaplyLoaderReturn()
{
@public
    // We're just wrapping the object that does the work
    WhirlyKit::QuadLoaderReturnRef loadReturn;
}

@end
