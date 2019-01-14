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

#import "MaplyQuadImageLoader.h"
#import "QuadTileBuilder.h"
#import "MaplyQuadSampler_private.h"

@interface MaplyQuadImageLoaderBase()
{
@public
    NSObject<MaplyTileFetcher> *tileFetcher;
    NSObject<MaplyLoaderInterpreter> *loadInterp;
    
    WhirlyKitQuadTileBuilder * __weak builder;
    WhirlyKitQuadDisplayLayerNew * __weak layer;
    int minLevel,maxLevel;
    GLenum texType;
    WhirlyKit::SimpleIdentity shaderID;
    
    MaplyBaseViewController * __weak viewC;
    MaplyRenderTarget * __weak renderTarget;
    MaplyQuadSamplingLayer *samplingLayer;
}

@end
