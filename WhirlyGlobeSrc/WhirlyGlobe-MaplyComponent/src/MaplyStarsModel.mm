/*
 *  MaplyStarsModel.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 6/4/15.
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

#import <vector>
#import "MaplyStarsModel.h"
#import <WhirlyGlobe.h>
#import <AA+.h>

using namespace WhirlyKit;

typedef struct
{
    float mag;
    float ra,dec;
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

- (id)initWithFileName:(NSString *)fileName
{
    self = [super init];
    
    FILE *fp = fopen([fileName cStringUsingEncoding:NSASCIIStringEncoding],"r");
    if (!fp)
        return nil;

    // Mangitude x y z
    char line[1024];
    while (fgets(line, 1023, fp))
    {
        stars.resize(stars.size()+1);
        SingleStar &star = stars.back();
        sscanf(line, "%f %f %f",&star.ra, &star.dec, &star.mag);
    }
    
    return self;
}

- (void)setImage:(UIImage *)inImage
{
    image = inImage;
}

static const char *vertexShaderTriPoint =
"uniform mat4  u_mvpMatrix;"
"uniform float u_radius;"
""
"attribute vec3 a_position;"
"attribute float a_size;"
""
"varying vec4 v_color;"
""
"void main()"
"{"
"   v_color = vec4(1.0,1.0,1.0,1.0);"
"   gl_PointSize = a_size;"
"   gl_Position = u_mvpMatrix * vec4(a_position * u_radius,1.0);"
"}"
;

static const char *fragmentShaderTriPoint =
"precision lowp float;"
""
"varying vec4      v_color;"
""
"void main()"
"{"
"  gl_FragColor = v_color;"
"}"
;

static const char *fragmentShaderTexTriPoint =
"precision lowp float;"
""
"uniform sampler2D s_baseMap0;"
"varying vec4      v_color;"
""
"void main()"
"{"
"  gl_FragColor = v_color * texture2D(s_baseMap0, gl_PointCoord);"
"}"
;

typedef struct
{
    float x,y,z;
} SimpleVec3;

- (void)addToViewC:(WhirlyGlobeViewController *)inViewC date:(NSDate *)date desc:(NSDictionary *)inDesc mode:(MaplyThreadMode)mode
{
    viewC = inViewC;
    addedMode = mode;

    // Julian date for position calculation
    NSCalendar *calendar = [[NSCalendar alloc] initWithCalendarIdentifier:NSGregorianCalendar];
    calendar.timeZone = [NSTimeZone timeZoneForSecondsFromGMT:0];
    NSDateComponents *components = [calendar components:(NSYearCalendarUnit | NSMonthCalendarUnit | NSDayCalendarUnit | NSHourCalendarUnit | NSMinuteCalendarUnit | NSSecondCalendarUnit) fromDate:date];
    CAADate aaDate(components.year,components.month,components.day,components.hour,components.minute,components.second,true);
    double jd = aaDate.Julian();
    double siderealTime = CAASidereal::MeanGreenwichSiderealTime(jd);

    // Really simple shader
    MaplyShader *shader = [[MaplyShader alloc] initWithName:@"Star Shader" vertex:[NSString stringWithFormat:@"%s",vertexShaderTriPoint] fragment:[NSString stringWithFormat:@"%s",(image ? fragmentShaderTexTriPoint : fragmentShaderTriPoint)] viewC:viewC];
    [viewC addShaderProgram:shader sceneName:@"Star Shader"];
    [shader setUniformFloatNamed:@"u_radius" val:6.0];
    
    NSMutableDictionary *desc = [NSMutableDictionary dictionaryWithDictionary:inDesc];
    if (!desc[kMaplyDrawPriority])
        desc[kMaplyDrawPriority] = @(kMaplyStarsDrawPriorityDefault);
    
    MaplyTexture *starTex = nil;
    if (image)
        starTex = [viewC addTexture:image imageFormat:MaplyImageIntRGBA wrapFlags:0 mode:MaplyThreadCurrent];

    // Set up a simple particle system (that doesn't move)
    partSys = [[MaplyParticleSystem alloc] initWithName:@"Stars"];
    partSys.type = MaplyParticleSystemTypePoint;
    partSys.lifetime = 1e20;
    partSys.totalParticles = stars.size();
    partSys.batchSize = stars.size();
    partSys.continuousUpdate = false;
    partSys.shader = shader.name;
    if (starTex)
        [partSys addTexture:starTex];
    [partSys addAttribute:@"a_position" type:MaplyShaderAttrTypeFloat3];
    [partSys addAttribute:@"a_size" type:MaplyShaderAttrTypeFloat];
    partSysObj = [viewC addParticleSystem:partSys desc:desc mode:mode];

    // Data arrays for particles
    // We'll clear them out in case we don't fill them out completely
    NSMutableData *posData = [[NSMutableData alloc] initWithLength:stars.size()*sizeof(SimpleVec3)];
    NSMutableData *sizeData = [[NSMutableData alloc] initWithLength:stars.size()*sizeof(float)];

    SimpleVec3 *posPtr = (SimpleVec3 *)[posData mutableBytes];
    float *magPtr = (float *)[sizeData mutableBytes];
    for (unsigned int ii=0;ii<stars.size();ii++)
    {
        SingleStar *star = &stars[ii];

        // Convert the start from equatorial to a useable lon/lat
        // Note: Should check this math
        double starLon = CAACoordinateTransformation::DegreesToRadians(star->ra-15*siderealTime);
        double starLat = CAACoordinateTransformation::DegreesToRadians(star->dec);
        
//        NSLog(@"star lon, lat = (%f,%f)",starLon*180/M_PI,starLat*180/M_PI);

//        Point3f pt;
        double z = sin(starLat);
        double rad = sqrt(1.0-z*z);
        Point3d pt(rad*cos(starLon),rad*sin(starLon),z);
        
//        pt.x() = cos(starLon);
//        pt.y() = sin(starLon);
//        pt.z() = sin(starLat);
//        pt.normalize();
        posPtr->x = pt.x();  posPtr->y = pt.y();  posPtr->z = pt.z();
        float mag = 6.0-star->mag;
        if (mag < 0.0)
            mag = 0.0;
        *magPtr = mag;
        
        posPtr++;   magPtr++;
    }

    // Set up the particle batch
    MaplyParticleBatch *batch = [[MaplyParticleBatch alloc] initWithParticleSystem:partSys];
    batch.time = CFAbsoluteTimeGetCurrent();
    [batch addAttribute:@"a_position" values:posData];
    [batch addAttribute:@"a_size" values:sizeData];
    [viewC addParticleBatch:batch mode:mode];
}

- (void)removeFromViewC
{
    if (partSysObj)
        [viewC removeObjects:@[partSysObj] mode:addedMode];
}

@end
