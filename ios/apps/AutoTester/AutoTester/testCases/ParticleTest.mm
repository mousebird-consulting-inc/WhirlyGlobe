/*
 *  ParticleTest.m
 *  WhirlyGlobeComponentTester
 *
 *  Created by Steve Gifford on 10/21/15.
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

#import "ParticleTest.h"

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

// Position calculation shader
static const char *vertexPositionShader = R"(
precision highp float;

uniform float u_time;

attribute vec3 a_position;
attribute vec3 a_dir;
attribute float a_startTime;

attribute float a_savedStartTime;
attribute vec3 a_savedPosition;

varying vec3 va_position;
varying float va_startTime;

void main()
{
    // Nudge it forward for 1/30th of a second
    // Doing this to test the saving
    vec3 thePos;
    if (a_savedStartTime < a_startTime)
        thePos = a_position;
    else {
        // An empty particle.  We'll just discard it.
        if (a_position == vec3(0.0,0.0,0.0))
            thePos = vec3(0.0,0.0,0.0);
        else
            thePos = normalize(a_savedPosition + 1.0/30.0*a_dir);
    }

    va_startTime = a_startTime;
    va_position = thePos;
}
)";

// The fragment shader is never used for position calculation
static const char *fragmentPositionShader = R"(
precision highp float;

void main()
{
}
)";

// Vertex shader for point version of particles
static const char *vertexRenderShaderPoint = R"(
precision highp float;

uniform mat4  u_mvpMatrix;
uniform mat4  u_mvMatrix;
uniform mat4  u_mvNormalMatrix;
uniform float u_size;
uniform float u_time;

attribute vec3 a_savedPosition;
attribute vec4 a_color;

varying vec4 v_color;

void main()
{
    v_color = a_color;
    vec3 thePos = a_savedPosition;
    // Convert from model space into display space
    vec4 pt = u_mvMatrix * vec4(thePos,1.0);
    pt /= pt.w;
    // Make sure the object is facing the user
    vec4 testNorm = u_mvNormalMatrix * vec4(thePos,0.0);
    float dot_res = dot(-pt.xyz,testNorm.xyz);
    // Set the point size
    gl_PointSize = u_size;
    // Project the point into 3-space
    gl_Position = (dot_res > 0.0 && thePos != vec3(0.0,0.0,0.0)) ? u_mvpMatrix * vec4(thePos,1.0) : vec4(1000.0,1000.0,1000.0,0.0);
}
)";

static const char *fragmentRenderShader = R"(
precision highp float;

varying vec4      v_color;

void main()
{
  gl_FragColor = v_color;
}
)";

// Vertex shader for rectangle version of particles
static const char *vertexRenderShaderRect = R"(
precision highp float;

uniform mat4  u_mvpMatrix;
uniform mat4  u_mvMatrix;
uniform mat4  u_mvNormalMatrix;
uniform float u_size;
uniform float u_len;

// Pixel size in model coordinates
uniform vec2 u_pixDispSize;

// Texture coordinate and offset for vertices within a rectangle
attribute vec2 a_texCoord;
attribute vec2 a_offset;

attribute vec3 a_savedPosition;
attribute vec3 a_position;
attribute vec3 a_dir;
attribute vec4 a_color;

varying vec4 v_color;

void main()
{
    vec3 thePos = a_savedPosition;
    vec3 dir = normalize(a_dir);
    float pixDispScale = min(u_pixDispSize.x,u_pixDispSize.y);
    float size = 2.0;
    float len = 8.0;

    // Convert from model space into display space
    // We'll use this for testing, rather than the actual point
    // This ensures we drop the whole particle at once
    vec4 pt = u_mvMatrix * vec4(thePos,1.0);
    pt /= pt.w;
    
    // Make sure the object is facing the user
    vec4 testNorm = u_mvNormalMatrix * vec4(thePos,0.0);
    float dot_res = dot(-pt.xyz,testNorm.xyz);
    
    vec3 dir0 = normalize(cross(dir,thePos));
    vec3 adjPos = a_offset.x * dir0 * pixDispScale * size + a_offset.y * dir * pixDispScale * len + thePos;
    
    // Output color and position
    v_color = a_color;
    gl_Position = (dot_res > 0.0 && thePos != vec3(0.0,0.0,0.0)) ? u_mvpMatrix * vec4(adjPos,1.0) : vec4(1000.0,1000.0,1000.0,-1000.0);
}
)";

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
    
    MaplyParticleSystemType partSysType = MaplyParticleSystemTypeRectangle;
    
    // These govern how the particles are structured
    _updateInterval = 0.05;
    _particleLifetime = 2.0;
    _numParticles = 100000/10;
    velocityScale = 0.01f;
    
    // Colors we'll use
    numVelocityColors = 3;
    velocityColors[0].r = 0.6f;  velocityColors[0].g = 1.0f;  velocityColors[0].b = 0.6f;  velocityColors[0].a = 1.f;
    velocityColors[1].r = 0.6f;  velocityColors[1].g = 0.6f;  velocityColors[1].b = 1.f;  velocityColors[1].a = 1.f;
    velocityColors[2].r = 1.f;  velocityColors[2].g = 0.6f;  velocityColors[2].b = 0.6f;  velocityColors[2].a = 1.f;
    
    // Position calculation shader
    MaplyShader *posShader = [[MaplyShader alloc] initWithViewC:viewC];
    [posShader addVarying:@"va_startTime"];
    [posShader addVarying:@"va_position"];
    [posShader delayedSetupWithName:@"Particle Wind Test Pos"
                             vertex:[NSString stringWithFormat:@"%s",vertexPositionShader]
                           fragment:[NSString stringWithFormat:@"%s",fragmentPositionShader]];
    [viewC addShaderProgram:posShader];
    
    // Render shader
    MaplyShader *renderShader = nil;
    if (partSysType == MaplyParticleSystemTypeRectangle) {
        renderShader = [[MaplyShader alloc] initWithName:@"Particle Wind Test Render Rects"
                                                  vertex:[NSString stringWithFormat:@"%s",vertexRenderShaderRect]
                                                fragment:[NSString stringWithFormat:@"%s",fragmentRenderShader]
                                                   viewC:viewC];
        [renderShader setUniformFloatNamed:@"u_len" val:8.0];
    } else {
        renderShader = [[MaplyShader alloc] initWithName:@"Particle Wind Test Render Points"
                                                     vertex:[NSString stringWithFormat:@"%s",vertexRenderShaderPoint]
                                                   fragment:[NSString stringWithFormat:@"%s",fragmentRenderShader]
                                                      viewC:viewC];
    }
    [viewC addShaderProgram:renderShader];
    
    // Set up the particle system we'll feed with particles
    partSys = [[MaplyParticleSystem alloc] initWithName:@"Particle Wind Test"];
    partSys.type = partSysType;
    partSys.positionShader = posShader;
    partSys.renderShader = renderShader;
    partSys.continuousUpdate = true;
    [partSys addAttribute:@"a_position" type:MaplyShaderAttrTypeFloat3];
    [partSys addAttribute:@"a_dir" type:MaplyShaderAttrTypeFloat3];
    [partSys addAttribute:@"a_color" type:MaplyShaderAttrTypeFloat4];
    [partSys addAttribute:@"a_startTime" type:MaplyShaderAttrTypeFloat];
    [partSys addVarying:@"va_startTime" inputName:@"a_savedStartTime" type:MaplyShaderAttrTypeFloat];
    [partSys addVarying:@"va_position" inputName:@"a_savedPosition" type:MaplyShaderAttrTypeFloat3];

    // Used to keep track of the tiles for fast lookup
    tileTrack = [[MaplyQuadTracker alloc] initWithViewC:(WhirlyGlobeViewController *)inViewC];
    tileTrack.minLevel = inMinZoom;
    tileTrack.coordSys = self.coordSys;

    locs = NULL;
    dirs = NULL;
    colors = NULL;
    
    cachedTiles = [NSMutableDictionary dictionary];
    
    // We need to refresh the particles periodically.  We'll do that one a single queue.
    queue = dispatch_queue_create("Wind Delegate",DISPATCH_QUEUE_SERIAL);

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
            self->cachedTiles[tileStr] = tile;
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

		// TODO if insecure certificates need to be supported, follow this:
		// http://stackoverflow.com/questions/20230169/nsurlsession-server-with-self-signed-cert

		[[[NSURLSession sharedSession] dataTaskWithURL:[NSURL URLWithString:urlStr]
				completionHandler: ^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {
					dispatch_async(self->queue, ^{
						if (error || !data) {
							[self clearTile:tileID];
							[layer tileFailedToLoad:tileID];
						}
						else {
							UIImage *img = [UIImage imageWithData:data];

							if (img)
							{
								DataTile *tile = [self getDataTile:tileID];

								// Happens if the tile is removed before the request comes back
								if (!tile)
									return;

								[tile setImage:img which:ii];

								if ([tile isComplete])
								{
									[self->tileTrack addTile:tileID];
									[layer tileDidLoad:tileID];
								}
							}
						}
					});
				}] resume];
    }
}

- (void)tileDidUnload:(MaplyTileID)tileID
{
    dispatch_async(queue,
    ^{
       [self->tileTrack removeTile:tileID];
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
    
    NSTimeInterval now = scene->getCurrentTime();

    // Data arrays for particles
    // These have to be raw data, rather than objects for speed
    int batchSize = partSys.batchSize;
    if (!locs)
    {
        locs = (SimpleLoc *)malloc(sizeof(SimpleLoc)*batchSize);
        dirs = (SimpleLoc *)malloc(sizeof(SimpleLoc)*batchSize);
        colors = (SimpleColor *)malloc(sizeof(SimpleColor)*batchSize);
        times = (float *)malloc(sizeof(float)*batchSize);
    }
    memset(locs, 0, batchSize*sizeof(SimpleLoc));
    memset(times, 0, batchSize*sizeof(float));
    memset(dirs, 0, batchSize*sizeof(SimpleLoc));
    memset(colors, 0, batchSize*sizeof(SimpleColor));

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
    MaplyQuadTrackerPointReturn *points = (MaplyQuadTrackerPointReturn *)malloc(sizeof(MaplyQuadTrackerPointReturn)*partSys.batchSize);
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
