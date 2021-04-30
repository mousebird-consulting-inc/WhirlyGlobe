/*  MaplyStarsModel.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 6/4/15.
 *  Copyright 2011-2021 mousebird consulting
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

#import <vector>
#import "visual_objects/MaplyStarsModel.h"
#import <WhirlyGlobe_iOS.h>
#import <AA+.h>
#import "WhirlyGlobeViewController_private.h"

using namespace WhirlyKit;

typedef struct
{
    float mag;  // atronomical magnitude (inverse brightness)
    float ra;   // right ascension (angle measured eastward along the celestial equator)
    float dec;  // declination (angle north or south of the celestial equator)
} SingleStar;

@implementation MaplyStarsModel
{
    std::vector<SingleStar> stars;
    MaplyParticleSystem *partSys;
    MaplyComponentObject *partSysObj;
    WhirlyGlobeViewController *__weak viewC;
    MaplyThreadMode addedMode;
    UIImage *image;
}

- (instancetype)initWithFileName:(NSString *)fileName
{
    self = [super init];
    
    FILE *fp = fopen([fileName cStringUsingEncoding:NSASCIIStringEncoding],"r");
    if (!fp)
        return nil;

    // Mangitude x y z
    char line[1024] = {0};
    while (fgets(line, 1023, fp))
    {
        stars.resize(stars.size()+1);
        SingleStar &star = stars.back();
        if (sscanf(line, "%f %f %f",&star.ra, &star.dec, &star.mag) != 3)
        {
            NSLog(@"Unrecognized: '%s'", line);
            stars.resize(stars.size()-1);
        }
    }
    
    return self;
}

- (void)setImage:(UIImage *)inImage
{
    image = inImage;
}

static const char *shaderCode = R"(

struct VertexIn {
    packed_float3 a_position;
    float         a_size;
};

struct VertexOut {
    float4 computedPosition [[position]];
    float4 color;
};

vertex VertexOut vert(constant VertexIn* vertex_array [[ buffer(0) ]],
                      unsigned int vid [[ vertex_id ]]) {
    VertexIn v = vertex_array[vid];
    VertexOut outVertex = VertexOut();
    outVertex.computedPosition = float4(v.a_position, 1.0);
    outVertex.color = float4(1,1,1,1);
    return outVertex;
}

//precision highp float;
//uniform mat4  u_mvpMatrix;
//uniform float u_radius;
//attribute vec3 a_position;
//attribute float a_size;
//varying vec4 v_color;
//void main()
//{
//   v_color = vec4(1.0,1.0,1.0,1.0);
//   gl_PointSize = a_size;
//   gl_Position = u_mvpMatrix * vec4(a_position * u_radius,1.0);
//}

fragment float4 frag(VertexOut interpolated [[stage_in]]) {
  return float4(interpolated.color);
}
//precision highp float;
//varying vec4      v_color;
//void main() {
//  gl_FragColor = v_color;
//}

fragment float4 fragTex(VertexOut interpolated [[stage_in]]) {
  return float4(interpolated.color);
}
//precision highp float;
//uniform sampler2D s_baseMap0;
//varying vec4      v_color;
//void main() {
//  gl_FragColor = v_color * texture2D(s_baseMap0, gl_PointCoord);
//}
)";

typedef struct
{
    float x,y,z;
} SimpleVec3;

- (bool)addToViewC:(WhirlyGlobeViewController *)inViewC date:(NSDate *)date desc:(NSDictionary *)inDesc mode:(MaplyThreadMode)mode
{
    viewC = inViewC;
    addedMode = mode;

    // Julian date for position calculation
    NSCalendar *calendar = [[NSCalendar alloc] initWithCalendarIdentifier:NSCalendarIdentifierGregorian];
    calendar.timeZone = [NSTimeZone timeZoneForSecondsFromGMT:0];
    NSDateComponents *components = [calendar components:(NSCalendarUnitYear | NSCalendarUnitMonth | NSCalendarUnitDay | NSCalendarUnitHour | NSCalendarUnitMinute | NSCalendarUnitSecond) fromDate:date];
    CAADate aaDate(components.year,components.month,components.day,components.hour,components.minute,components.second,true);
    double jd = aaDate.Julian();
    double siderealTime = CAASidereal::MeanGreenwichSiderealTime(jd);

    //id<MTLDevice> mtlDevice = MTLCreateSystemDefaultDevice();
    const auto mtlLib = [inViewC getMetalLibrary];
    MTLCompileOptions *mtlOpt = [[MTLCompileOptions alloc] init];
    mtlOpt.fastMathEnabled = true;
    //mtlOpt.preserveInvariance = true;
    //mtlOpt.languageVersion =
    NSError *mtlErr = nil;
    id<MTLFunction> vertexFunc = [mtlLib newFunctionWithName:@"vertStars"];
    id<MTLFunction> fragmentFunc = [mtlLib newFunctionWithName:@"fragmentStars"];
    if (!vertexFunc || !fragmentFunc)
    {
        NSLog(@"Failed to create star model vertex library: %@", mtlErr);
        return false;
    }

    MaplyShader *shader = [[MaplyShader alloc] initMetalWithName:@"Star Shader"
                                                          vertex:vertexFunc
                                                        fragment:fragmentFunc
                                                           viewC:inViewC];
    if (shader)
    {
        [inViewC addShaderProgram:shader];
        //[shader setUniformBlock:<#(NSData * _Nonnull)#> buffer:<#(int)#>]
        //[shader setUniformFloatNamed:@"u_radius" val:6.0];
    }

    NSMutableDictionary *desc = [NSMutableDictionary dictionaryWithDictionary:inDesc];
    if (!desc[kMaplyDrawPriority])
        desc[kMaplyDrawPriority] = @(kMaplyStarsDrawPriorityDefault);
    
    MaplyTexture *starTex = nil;
    if (image)
        starTex = [inViewC addTexture:image imageFormat:MaplyImageIntRGBA wrapFlags:0 mode:MaplyThreadCurrent];

    // Set up a simple particle system (that doesn't move)
    partSys = [[MaplyParticleSystem alloc] initWithName:@"Stars" viewC:inViewC];
    partSys.type = MaplyParticleSystemTypePoint;
    partSys.lifetime = 1e20;
    partSys.totalParticles = (int)stars.size();
    partSys.batchSize = (int)stars.size();
    partSys.continuousUpdate = false;
    partSys.renderShader = shader;
    partSys.vertexSize = sizeof(SimpleVec3)+sizeof(float); // ?
    if (starTex)
        [partSys addTexture:starTex];
    [partSys addAttribute:@"a_position" type:MaplyShaderAttrTypeFloat3];
    [partSys addAttribute:@"a_size" type:MaplyShaderAttrTypeFloat];
    partSysObj = [inViewC addParticleSystem:partSys desc:desc mode:mode];

    // Data arrays for particles
    // We'll clear them out in case we don't fill them out completely
    NSMutableData *posData = [[NSMutableData alloc] initWithLength:stars.size()*sizeof(SimpleVec3)];
    NSMutableData *sizeData = [[NSMutableData alloc] initWithLength:stars.size()*sizeof(float)];

    SimpleVec3 *posPtr = (SimpleVec3 *)[posData mutableBytes];
    float *magPtr = (float *)[sizeData mutableBytes];
    for (const auto &star : stars)
    {
        // Convert the star from equatorial to a useable lon/lat
        // Note: Should check this math
        const double starLon = CAACoordinateTransformation::DegreesToRadians(star.ra-15*siderealTime);
        const double starLat = CAACoordinateTransformation::DegreesToRadians(star.dec);
        
//        NSLog(@"star lon, lat = (%f,%f)",starLon*180/M_PI,starLat*180/M_PI);

        const double z = sin(starLat);
        const double rad = sqrt(1.0-z*z);
        posPtr->x = rad*cos(starLon);
        posPtr->y = rad*sin(starLon);
        posPtr->z = z;
        posPtr++;

        *magPtr = std::max(0.0f, 6.0f - star.mag);
        magPtr++;
    }

    // Set up the particle batch
    MaplyParticleBatch *batch = [[MaplyParticleBatch alloc] initWithParticleSystem:partSys];
    batch.time = inViewC->renderControl->scene->getCurrentTime();
    [batch addAttribute:@"a_position" values:posData];
    [batch addAttribute:@"a_size" values:sizeData];
    
    // batch object isn't populating its own data
    NSMutableData *batchData = [[NSMutableData alloc] initWithCapacity:posData.length + sizeData.length];
    [batchData appendData:posData];
    [batchData appendData:sizeData];
    [batch addData:batchData];

    [inViewC addParticleBatch:batch mode:mode];
    return true;
}

- (void)removeFromViewC
{
    if (partSysObj)
        [viewC removeObjects:@[partSysObj] mode:addedMode];
}

@end
