/*
 *  MaplyQuadSampler_private.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 6/15/18.
 *  Copyright 2011-2019 Saildrone Inc
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

#import "WhirlyGlobe.h"
#import "loading/MaplyQuadSampler.h"
#import "QuadTileBuilder.h"
#import "QuadDisplayLayerNew.h"
#import "MaplyControllerLayer_private.h"

@interface MaplySamplingParams()
{
@public
    WhirlyKit::SamplingParams params;
}

@end

/** The Quad Sampling Layer runs a quad tree which determines what
 tiles to load.  We hook up other things to this to actually do
 the loading.
 */
@interface MaplyQuadSamplingLayer : MaplyControllerLayer
{
@public
    WhirlyKit::QuadSamplingController sampleControl;
}

// Parameters this sampler is using (non changeable)
@property (nonatomic) WhirlyKit::SamplingParams params;

// Number of clients using this sampler
@property (nonatomic,readonly) int numClients;

@property (nonatomic) bool debugMode;

// Initialize with the sampling parameters
- (nullable instancetype)initWithParams:(const WhirlyKit::SamplingParams &)params;

// Add a new builder delegate to watch tile related events
- (void)addBuilderDelegate:(WhirlyKit::QuadTileBuilderDelegateRef)delegate;

// Remove the given builder delegate that was watching tile related events
- (void)removeBuilderDelegate:(WhirlyKit::QuadTileBuilderDelegateRef)delegate;

// True if any of the delegates are loading
- (bool)isLoading;

@property (nonatomic) WhirlyKitQuadDisplayLayerNew * __nullable quadLayer;

@end
