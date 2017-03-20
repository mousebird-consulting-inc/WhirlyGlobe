/*
 *  ParticleTest.h
 *  WhirlyGlobeComponentTester
 *
 *  Created by Steve Gifford on 10/21/15.
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
#import <WhirlyGlobeComponent.h>


// Handles fetching and managing the particle source data tiles
@interface ParticleTileDelegate : NSObject<MaplyPagingDelegate>

@property(nonatomic,weak) MaplyQuadPagingLayer *layer;
@property(nonatomic,strong) MaplyCoordinateSystem *coordSys;
@property(nonatomic) int minZoom,maxZoom;
@property(nonatomic) double updateInterval;
@property(nonatomic) double particleLifetime;
@property(nonatomic) int numParticles;

// URL is of the form http://whatever/{dir}/{z}/{x}/{y} or similar
- (id)initWithURL:(NSString *)url minZoom:(int)minZoom maxZoom:(int)maxZoom viewC:(MaplyBaseViewController *)inViewC;

@end
