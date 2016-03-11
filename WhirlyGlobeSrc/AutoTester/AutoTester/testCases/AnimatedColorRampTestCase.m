//
//  AnimatedColorRampTestCase.m
//  AutoTester
//
//  Created by Steve Gifford on 2/24/16.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

#import "AnimatedColorRampTestCase.h"
#import "GeographyClassTestCase.h"
#import <WhirlyGlobeComponent.h>

// Note: Rather than copying the shader code in here, we should have a way to look it up
static NSString *vertexShaderNoLightTri =
@"uniform mat4  u_mvpMatrix;                   \n"
"uniform float u_fade;                        \n"
"attribute vec3 a_position;                  \n"
"attribute vec2 a_texCoord0;                  \n"
"attribute vec4 a_color;                     \n"
"attribute vec3 a_normal;                    \n"
"\n"
"varying vec2 v_texCoord;                    \n"
"varying vec4 v_color;                       \n"
"\n"
"void main()                                 \n"
"{                                           \n"
"   v_texCoord = a_texCoord0;                 \n"
"   v_color = a_color * u_fade;\n"
"\n"
"   gl_Position = u_mvpMatrix * vec4(a_position,1.0);  \n"
"}                                           \n"
;

static NSString *fragmentShaderTriMultiTexRamp =
@"precision mediump float;\n"
"\n"
"uniform sampler2D s_baseMap0;\n"
"uniform sampler2D s_baseMap1;\n"
"uniform sampler2D s_colorRamp;\n"
"uniform float u_interp;\n"
"\n"
"varying vec2      v_texCoord;\n"
"varying vec4      v_color;\n"
"\n"
"void main()\n"
"{\n"
"  float baseVal0 = texture2D(s_baseMap0, v_texCoord).a;\n"
"  float baseVal1 = texture2D(s_baseMap1, v_texCoord).a;\n"
"  float index = mix(baseVal0,baseVal1,u_interp);\n"
"  gl_FragColor = texture2D(s_colorRamp,vec2(index,0.5));\n"
"}\n"
;

@implementation AnimatedColorRampTestCase

- (instancetype)init
{
    if (self = [super init]) {
        self.name = @"Animated Color Ramp";
        self.captureDelay = 10;
    }
    
    return self;
}

- (void)setupWeatherLayer:(MaplyBaseViewController *)viewC
{
    UIImage *colorRamp = [UIImage imageNamed:@"colorramp.png"];
    MaplyShader *shader = [[MaplyShader alloc] initWithName:@"Color Ramp Test Shader" vertex:vertexShaderNoLightTri fragment:fragmentShaderTriMultiTexRamp viewC:viewC];
    [viewC addShaderProgram:shader sceneName:@"Color Ramp Test Shader"];
    [shader addTextureNamed:@"s_colorRamp" image:colorRamp];
    
    // For network paging layers, where we'll store temp files
    NSString *cacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)  objectAtIndex:0];

    // Collect up the various precipitation sources
    NSMutableArray *tileSources = [NSMutableArray array];
    for (unsigned int ii=0;ii<5;ii++)
    {
        MaplyRemoteTileInfo *precipTileSource =
        [[MaplyRemoteTileInfo alloc]
         initWithBaseURL:[NSString stringWithFormat:@"http://a.tiles.mapbox.com/v3/mousebird.precip-example-layer%d/",ii] ext:@"png" minZoom:0 maxZoom:6];
        precipTileSource.cacheDir = [NSString stringWithFormat:@"%@/forecast_io_weather_layer%d/",cacheDir,ii];
        [tileSources addObject:precipTileSource];
    }
    MaplyMultiplexTileSource *precipTileSource = [[MaplyMultiplexTileSource alloc] initWithSources:tileSources];
    // Create a precipitation layer that animates
    MaplyQuadImageTilesLayer *precipLayer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:precipTileSource.coordSys tileSource:precipTileSource];
    precipLayer.imageDepth = (int)[tileSources count];
    precipLayer.animationPeriod = 6.0;
    precipLayer.imageFormat = MaplyImageUByteRed;
    precipLayer.handleEdges = false;
    precipLayer.coverPoles = false;
    precipLayer.shaderProgramName = shader.name;

    [viewC addLayer:precipLayer];
}

- (BOOL)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
    GeographyClassTestCase *baseView = [[GeographyClassTestCase alloc]init];
    [baseView setUpWithGlobe:globeVC];
    [self setupWeatherLayer:globeVC];
    
    return true;
}


- (BOOL)setUpWithMap:(MaplyViewController *)mapVC
{
    GeographyClassTestCase *baseView = [[GeographyClassTestCase alloc]init];
    [baseView setUpWithMap:mapVC];
    [self setupWeatherLayer:mapVC];
    
    return true;
}

@end
