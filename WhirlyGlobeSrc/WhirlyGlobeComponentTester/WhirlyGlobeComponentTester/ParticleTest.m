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
#import "AFHTTPRequestOperation.h"

typedef struct
{
    float x,y,z;
} SimpleLoc;

typedef struct
{
    float r,g,b,a;
} SimpleColor;

// Used to store the data for a single tile
@interface DataTile : NSObject

@property (nonatomic,assign) MaplyTileID tileID;

@end

@implementation DataTile
{
    NSMutableArray *tiles;
    int pixSizeX,pixSizeY;
}

// Tile starts empty
- (id)init
{
    self = [super init];
    
    tiles = [NSMutableArray array];
    [tiles addObject:[NSNull null]];
    [tiles addObject:[NSNull null]];
    
    return self;
}

// Tile is completely loaded (and thus usable)
- (bool)isComplete
{
    for (unsigned int ii=0;ii<2;ii++)
        if ([tiles[ii] isKindOfClass:[NSNull class]])
            return false;
    
    return true;
}

- (void)setImage:(UIImage *)img which:(int)which
{
    pixSizeX = img.size.width;
    pixSizeY = img.size.height;
    
    NSData *rawData = [self rawData:img];
    tiles[which] = rawData;
}

// Dump the contents of the image out as raw pixel data
-(NSData *)rawData:(UIImage *)image
{
    CGImageRef cgImage = image.CGImage;
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceGray();
    
    NSMutableData *retData = [NSMutableData dataWithLength:(pixSizeX)*(pixSizeY)*1];
    CGContextRef theContext = CGBitmapContextCreate((void *)[retData bytes], (pixSizeX), (pixSizeY), 8, (pixSizeX) * 1, colorSpace, kCGBitmapByteOrderDefault);
    CGContextDrawImage(theContext, CGRectMake(0.0, 0.0, (CGFloat)(pixSizeX), (CGFloat)(pixSizeY)), cgImage);
    CGContextRelease(theContext);
    CGColorSpaceRelease(colorSpace);
    
    return retData;
}

- (bool)getValue:(CGPoint)pt u:(unsigned char *)u v:(unsigned char *)v
{
    if (![self isComplete])
        return false;
    
    pt.y = 1.0 - pt.y;
    
    int whereX = pt.x*pixSizeX, whereY = pt.y*pixSizeY;
    whereX = MAX(0,whereX);  whereY = MAX(0,whereY);
    whereX = MIN(255,whereX);  whereY = MIN(255,whereY);
    
    *u = ((unsigned char *)[[tiles objectAtIndex:0] bytes])[whereY*pixSizeY + whereX];
    *v = ((unsigned char *)[[tiles objectAtIndex:1] bytes])[whereY*pixSizeY + whereX];
    
    return true;
}

@end

@implementation ParticleTileDelegate
{
    NSString *url;
    MaplyParticleSystem *partSys;
    MaplyBaseViewController * __weak viewC;
    dispatch_queue_t queue;
    MaplyComponentObject *partSysObj;
    SimpleLoc *locs,*dirs;
    SimpleColor *colors;
    float *times;
    NSMutableDictionary *cachedTiles;
    MaplyQuadTracker *tileTrack;
    float velocityScale;
    int numVelocityColors;
    SimpleColor velocityColors[3];
}

- (id)initWithURL:(NSString *)inURL minZoom:(int)inMinZoom maxZoom:(int)inMaxZoom viewC:(MaplyBaseViewController *)inViewC
{
    self = [super init];
    url = inURL;
    _minZoom = inMinZoom;
    _maxZoom = inMaxZoom;
    _coordSys = [[MaplySphericalMercator alloc] initWebStandard];
    viewC = inViewC;
    
    // These govern how the particles are structured
    _updateInterval = 0.05;
    _particleLifetime = 2.0;
    _numParticles = 100000;
    velocityScale = 0.01f;
    
    // Colors we'll use
    numVelocityColors = 3;
    velocityColors[0].r = 0.6f;  velocityColors[0].g = 1.0f;  velocityColors[0].b = 0.6f;  velocityColors[0].a = 1.f;
    velocityColors[1].r = 0.6f;  velocityColors[1].g = 0.6f;  velocityColors[1].b = 1.f;  velocityColors[1].a = 1.f;
    velocityColors[2].r = 1.f;  velocityColors[2].g = 0.6f;  velocityColors[2].b = 0.6f;  velocityColors[2].a = 1.f;
    
    // Set up the particle system we'll feed with particles
    partSys = [[MaplyParticleSystem alloc] initWithName:@"Particle Wind Test"];
    partSys.type = MaplyParticleSystemTypePoint;
    partSys.shader = kMaplyShaderParticleSystemPointDefault;
    [partSys addAttribute:@"a_position" type:MaplyShaderAttrTypeFloat3];
    [partSys addAttribute:@"a_dir" type:MaplyShaderAttrTypeFloat3];
    [partSys addAttribute:@"a_color" type:MaplyShaderAttrTypeFloat4];
    [partSys addAttribute:@"a_startTime" type:MaplyShaderAttrTypeFloat];

    // Used to keep track of the tiles for fast lookup
    tileTrack = [[MaplyQuadTracker alloc] initWithViewC:(WhirlyGlobeViewController *)inViewC];
    tileTrack.minLevel = inMinZoom;
    tileTrack.coordSys = self.coordSys;

    locs = NULL;
    dirs = NULL;
    colors = NULL;
    
    cachedTiles = [NSMutableDictionary dictionary];
    
    // We need to refresh the particles periodically.  We'll do that one a single queue.
    queue = dispatch_queue_create("Wind Delegate",NULL);

    // Kick off the first generation of particles
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(_updateInterval * NSEC_PER_SEC)), queue,
        ^{
           [self generateParticles];
        });
    
    return self;
}

- (void)dealloc
{
    if (locs)
    {
        free(locs);
        free(dirs);
        free(colors);
        free(times);
    }
}

- (void)startFetchForTile:(MaplyTileID)tileID forLayer:(MaplyQuadPagingLayer *)layer
{
    // Make sure there's an entry for the tiles
    dispatch_async(queue,
    ^{
        DataTile *tile = [self getDataTile:tileID];
        if (!tile)
        {
            NSString *tileStr = [self indexForTile:tileID];
            tile = [[DataTile alloc] init];
            tile.tileID = tileID;
            cachedTiles[tileStr] = tile;
        }
    });
    
    // Ask for both sets of tiles
    for (unsigned int ii=0;ii<2;ii++)
    {
        NSString *uOrV = (ii == 0 ? @"u" : @"v");
        NSString *xStr = [NSString stringWithFormat:@"%d",tileID.x];
        NSString *yStr = [NSString stringWithFormat:@"%d",tileID.y];
        NSString *zStr = [NSString stringWithFormat:@"%d",tileID.level];
        NSString *urlStr = [[[[url stringByReplacingOccurrencesOfString:@"{dir}" withString:uOrV] stringByReplacingOccurrencesOfString:@"{z}" withString:zStr] stringByReplacingOccurrencesOfString:@"{x}" withString:xStr] stringByReplacingOccurrencesOfString:@"{y}" withString:yStr];
        NSMutableURLRequest *urlReq = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:urlStr]];
        
        AFHTTPRequestOperation *operation = [[AFHTTPRequestOperation alloc] initWithRequest:urlReq];
        operation.securityPolicy.allowInvalidCertificates = true;
        // Need to process the tile on our own queue
        operation.completionQueue = queue;
        [operation
         setCompletionBlockWithSuccess:^(AFHTTPRequestOperation *operation, NSData *respData)
         {
             UIImage *img = [UIImage imageWithData:respData];

             if (img)
             {
                 DataTile *tile = [self getDataTile:tileID];

                 // Happens if the tile is removed before the request comes back
                 if (!tile)
                     return;
                 
                 [tile setImage:img which:ii];
                 
                 if ([tile isComplete])
                 {
                     [tileTrack addTile:tileID];
                     [layer tileDidLoad:tileID];
                 }
             }
         }
         failure:^(AFHTTPRequestOperation *operation, NSError *error)
         {
             [self clearTile:tileID];
             [layer tileFailedToLoad:tileID];
         }
         ];
        [operation start];
    }
}

- (void)tileDidUnload:(MaplyTileID)tileID
{
    dispatch_async(queue,
    ^{
       [tileTrack removeTile:tileID];
       [self clearTile:tileID];
    });
}

- (NSString *)indexForTile:(MaplyTileID)tileID
{
    char tileStr[100];
    // Letting NSString do this is sloooow
    sprintf(tileStr,"%d_%d_%d",tileID.x,tileID.y,tileID.level);
    return [[NSString alloc] initWithUTF8String:tileStr];
}

- (DataTile *)getDataTile:(MaplyTileID)tileID
{
    DataTile *dataTile = [cachedTiles objectForKey:[self indexForTile:tileID]];
    
    return dataTile;
}

- (void)clearTile:(MaplyTileID)tileID
{
    [cachedTiles removeObjectForKey:[self indexForTile:tileID]];
}

static const float sqrt2 = 1.41421356237;

// Interpolate a color based on the velocity of the particle
- (void)color:(SimpleColor *)color forVel:(float)vel
{
    vel = MAX(0.f,vel);
    vel = MIN(1.f,vel);
    
    SimpleColor *c0,*c1;
    if (vel < 0.5)
    {
        c0 = &velocityColors[0];
        c1 = &velocityColors[1];
    } else {
        c0 = &velocityColors[1];
        c1 = &velocityColors[2];
    }
    
    color->r = (c1->r - c0->r)*vel + c0->r;
    color->g = (c1->g - c0->g)*vel + c0->g;
    color->b = (c1->b - c0->b)*vel + c0->b;
    color->a = (c1->a - c0->a)*vel + c0->a;
}

- (void)generateParticles
{
    // Add the 
    if (!partSysObj)
    {
        partSys.lifetime = _particleLifetime;
        partSys.totalParticles = _numParticles;
        partSys.batchSize = (_numParticles / (_particleLifetime/_updateInterval));
        partSysObj = [viewC addParticleSystem:partSys desc:@{kMaplyPointSize: @(4.0), kMaplyDrawPriority: @(kMaplyModelDrawPriorityDefault+1000)} mode:MaplyThreadCurrent];
    }
    
    NSTimeInterval now = CFAbsoluteTimeGetCurrent();

    // Data arrays for particles
    // These have to be raw data, rather than objects for speed
    int batchSize = partSys.batchSize;
    if (!locs)
    {
        locs = malloc(sizeof(SimpleLoc)*batchSize);
        dirs = malloc(sizeof(SimpleLoc)*batchSize);
        colors = malloc(sizeof(SimpleColor)*batchSize);
        times = malloc(sizeof(float)*batchSize);
    }
    memset(locs, 0, batchSize*sizeof(SimpleLoc));
    memset(times, 0, batchSize*sizeof(float));

    // Make up some random particles
#if 0
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
        sum = sqrtf(dir->x*dir->x + dir->y*dir->y + dir->z*dir->z);
        dir->x /= sum;  dir->y /= sum;  dir->z /= sum;

        color->r = 1.0;  color->g = 1.0;  color->b = 1.0;  color->a = 1.0;
    }
#endif

    // Generate some screen coordinates for sampling
    MaplyQuadTrackerPointReturn *points = malloc(sizeof(MaplyQuadTrackerPointReturn)*partSys.batchSize);
    MaplyQuadTrackerPointReturn *pt = points;
    for (unsigned int ii=0;ii<partSys.batchSize;ii++)
    {
        pt->screenU = drand48();
        pt->screenV = drand48();
        pt++;
    }

    // Figure out which the samples show up in a tile on the earth
    // We do it this way so we're not wasting particles on parts of the globe that aren't visible
    [tileTrack tiles:points forPoints:partSys.batchSize];
    
    // Generate particles from those samples
    pt = points;
    int whichPart = 0;
    for (unsigned int ii=0;ii<partSys.batchSize;ii++)
    {
        // Look for the associated tile
        DataTile *dataTile = [self getDataTile:pt->tileID];
        
        if (dataTile)
        {
            SimpleLoc *loc = &locs[whichPart];
            SimpleLoc *dir = &dirs[whichPart];
            SimpleColor *color = &colors[whichPart];
            float *time = &times[whichPart];
            *time = now-partSys.baseTime;
            
            MaplyCoordinate3d coordA;
            coordA.x = pt->locX;
            coordA.y = pt->locY;
            coordA.z = 0.0;
            
            float velU = 0.0, velV = 0.0;
            unsigned char u=0,v=0;
            if ([dataTile getValue:CGPointMake(pt->tileU, pt->tileV) u:&u v:&v])
            {
                // There are a lot of empty values in the data so we'll skip those
                if (u != 0 || v != 0)
                {
                    velU = (u-127.0)/128.0;
                    velV = (v-127.0)/128.0;
                    float vel = sqrtf(velU*velU+velV*velV)/sqrt2 * 3;
                    
                    MaplyCoordinate3d coordB;
                    coordB.x = coordA.x + velU * velocityScale;
                    coordB.y = coordA.y + velV * velocityScale;
                    coordB.z = 0.0;
                    
                    // Convert to display coordinates
                    MaplyCoordinate3d dispA = [viewC displayCoord:coordA fromSystem:_coordSys];
                    MaplyCoordinate3d dispB = [viewC displayCoord:coordB fromSystem:_coordSys];
                    MaplyCoordinate3d calcDir = MaplyCoordinate3dMake(dispB.x-dispA.x, dispB.y-dispA.y, dispB.z-dispA.z);
                    
                    loc->x = dispA.x;  loc->y = dispA.y;  loc->z = dispA.z;
                    dir->x = calcDir.x;  dir->y = calcDir.y;  dir->z = calcDir.z;
                    
                    // Calculate a color based on the velocity
                    [self color:color forVel:vel];
                    
                    whichPart++;
                }
            }
        }
        
        pt++;
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
    NSData *timeData = [[NSData alloc] initWithBytesNoCopy:times length:partSys.batchSize*sizeof(float) freeWhenDone:false];
    [batch addAttribute:@"a_startTime" values:timeData];
    
    [viewC addParticleBatch:batch mode:MaplyThreadCurrent];
    
    // Kick off the next batch
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(_updateInterval * NSEC_PER_SEC)), queue,
                   ^{
                       [self generateParticles];
                   });
}

@end
