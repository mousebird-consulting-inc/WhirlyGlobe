/*
 *  ParticleTest.m
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

#import "ParticleTest.h"

typedef struct
{
    float x,y,z;
} SimpleLoc;

typedef struct
{
    float r,g,b,a;
} SimpleColor;

@implementation ParticleTileDelegate
{
    NSString *url;
    MaplyParticleSystem *partSys;
    MaplyBaseViewController * __weak viewC;
    dispatch_queue_t queue;
    MaplyComponentObject *partSysObj;
    SimpleLoc *locs,*dirs;
    SimpleColor *colors;
}

- (id)initWithURL:(NSString *)inURL minZoom:(int)inMinZoom maxZoom:(int)inMaxZoom viewC:(MaplyBaseViewController *)inViewC
{
    self = [super init];
    url = inURL;
    _minZoom = inMinZoom;
    _maxZoom = inMaxZoom;
    viewC = inViewC;
    _updateInterval = 0.1;
    _particleLifetime = 2.0;
    _numParticles = 500000;
    
    // Set up the particle system we'll feed with particles
    partSys = [[MaplyParticleSystem alloc] initWithName:@"Particle Wind Test"];
    partSys.type = MaplyParticleSystemTypePoint;
    partSys.shader = kMaplyShaderParticleSystemPointDefault;
    [partSys addAttribute:@"a_position" type:MaplyShaderAttrTypeFloat3];
    [partSys addAttribute:@"a_dir" type:MaplyShaderAttrTypeFloat3];
    [partSys addAttribute:@"a_color" type:MaplyShaderAttrTypeFloat4];

    locs = NULL;
    dirs = NULL;
    colors = NULL;
    
    // We need to refresh the particles periodically.  We'll do that one a single queue.
    queue = dispatch_queue_create("Wind Delegate",NULL);

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(_updateInterval * NSEC_PER_SEC)), queue,
        ^{
           [self generateParticles];
        });
    
    return self;
}

- (void)startFetchForTile:(MaplyTileID)tileID forLayer:(MaplyQuadPagingLayer *)layer
{
    [layer tileDidLoad:tileID];
}

- (void)generateParticles
{
    // Add the 
    if (!partSysObj)
    {
        partSys.lifetime = _particleLifetime;
        partSys.batchSize = (_numParticles / (_particleLifetime/_updateInterval));
        partSysObj = [viewC addParticleSystem:partSys desc:@{kMaplyPointSize: @(4.0), kMaplyDrawPriority: @(kMaplyModelDrawPriorityDefault+1000)} mode:MaplyThreadCurrent];
    }
    
    NSTimeInterval now = CFAbsoluteTimeGetCurrent();

    // Data arrays for particles
    // These have to be raw data, rather than objects for speed
    if (!locs)
    {
        int batchSize = partSys.batchSize;
        locs = malloc(sizeof(SimpleLoc)*batchSize);
        dirs = malloc(sizeof(SimpleLoc)*batchSize);
        colors = malloc(sizeof(SimpleColor)*batchSize);
    }

    // Make up some random particles
    for (unsigned int ii=0;ii<partSys.batchSize;ii++)
    {
        SimpleLoc *loc = &locs[ii];
        SimpleLoc *dir = &dirs[ii];
        SimpleColor *color = &colors[ii];

        // Random location
        loc->x = drand48()*2-1;  loc->y = drand48()*2-1;  loc->z = drand48()*2-1;
        float sum = sqrtf(loc->x*loc->x + loc->y*loc->y + loc->z*loc->z);
        loc->x /= sum;  loc->y /= sum;  loc->z /= sum;
        
        // Random direction
        dir->x = drand48()*2-1;  dir->y = drand48()*2-1;  dir->z = drand48()*2-1;
        sum = sqrtf(dir->x*dir->x + dir->y*dir->y + dir->z*dir->z)/100.0;
        dir->x /= sum;  dir->y /= sum;  dir->z /= sum;

        color->r = 1.0;  color->g = 1.0;  color->b = 1.0;  color->a = 1.0;
    }
    
    // Set up the batch
    MaplyParticleBatch *batch = [[MaplyParticleBatch alloc] initWithParticleSystem:partSys];
    batch.time = now;
    NSData *posData = [[NSData alloc] initWithBytesNoCopy:locs length:partSys.batchSize*sizeof(SimpleLoc) freeWhenDone:false];
    [batch addAttribute:@"a_position" values:posData];
    NSData *dirData = [[NSData alloc] initWithBytesNoCopy:dirs length:partSys.batchSize*sizeof(SimpleLoc) freeWhenDone:false];
    [batch addAttribute:@"a_dir" values:dirData];
    NSData *colorData = [[NSData alloc] initWithBytesNoCopy:colors length:partSys.batchSize*sizeof(SimpleColor) freeWhenDone:false];
    [batch addAttribute:@"a_color" values:colorData];
    
    [viewC addParticleBatch:batch mode:MaplyThreadCurrent];
    
    // Kick off the next batch
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(_updateInterval * NSEC_PER_SEC)), queue,
                   ^{
                       [self generateParticles];
                   });
}

@end
