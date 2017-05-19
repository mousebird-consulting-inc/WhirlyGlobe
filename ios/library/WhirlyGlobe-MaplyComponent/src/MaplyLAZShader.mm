//
//  LAZShader.m
//  LidarViewer
//
//  Created by Steve Gifford on 10/27/15.
//  Copyright © 2015-2017 mousebird consulting. All rights reserved.
//

#import "MaplyLAZShader.h"

NSString* const kMaplyLAZShaderPointSize = @"u_pointsize";
NSString* const kMaplyLAZShaderZMin = @"u_zmin";
NSString* const kMaplyLAZShaderZMax = @"u_zmax";

static const char *vertexShaderTriPoint =
"uniform mat4  u_mvpMatrix;\n"
"uniform float     u_pointsize;\n"
"\n"
"attribute vec3 a_position;\n"
"attribute vec4 a_color;\n"
"attribute float a_size;\n"
"\n"
"varying vec4 v_color;\n"
"\n"
"void main()\n"
"{\n"
"   v_color = a_color;\n"
"   gl_PointSize = u_pointsize;\n"
"   gl_Position = u_mvpMatrix * vec4(a_position,1.0);\n"
"}\n"
;

static const char *vertexShaderTriPointRamp =
"uniform mat4  u_mvpMatrix;\n"
"\n"
"uniform float     u_zmin;\n"
"uniform float     u_zmax;\n"
"uniform float     u_pointsize;\n"
"\n"
"attribute vec3 a_position;\n"
"attribute float a_size;\n"
"attribute float a_elev;\n"
"\n"
"varying float v_colorPos;\n"
"\n"
"void main()\n"
"{\n"
"   v_colorPos = (a_elev-u_zmin)/(u_zmax-u_zmin);\n"
"\n"
"   gl_PointSize = u_pointsize;\n"
"   gl_Position = u_mvpMatrix * vec4(a_position,1.0);\n"
"}\n"
;

static const char *fragmentShaderTriPoint =
"precision mediump float;\n"
"\n"
"varying vec4      v_color;\n"
"\n"
"void main()\n"
"{\n"
"  gl_FragColor = v_color;\n"
"}\n"
;

static const char *fragmentShaderTriPointRamp =
"precision mediump float;\n"
"\n"
"uniform sampler2D s_colorRamp;\n"
"\n"
"varying float v_colorPos;\n"
"\n"
"void main()\n"
"{\n"
"  vec4 color = texture2D(s_colorRamp,vec2(v_colorPos,0.5));\n"
"  gl_FragColor = color;\n"
"}\n"
;

MaplyShader *MaplyLAZBuildPointShader(MaplyBaseViewController *viewC)
{
    MaplyShader *shader = [[MaplyShader alloc] initWithName:@"Lidar Point Shader" vertex:[NSString stringWithFormat:@"%s",vertexShaderTriPoint] fragment:[NSString stringWithFormat:@"%s",fragmentShaderTriPoint] viewC:viewC];
    
    if (shader)
    {
        [viewC addShaderProgram:shader sceneName:shader.name];
        [shader setUniformFloatNamed:kMaplyLAZShaderPointSize val:6.0];
    }
    
    return shader;
}

MaplyShader *MaplyLAZBuildRampPointShader(MaplyBaseViewController *viewC,UIImage *colorRamp)
{
    MaplyShader *shader = [[MaplyShader alloc] initWithName:@"Lidar Ramp Point Shader" vertex:[NSString stringWithFormat:@"%s",vertexShaderTriPointRamp] fragment:[NSString stringWithFormat:@"%s",fragmentShaderTriPointRamp] viewC:viewC];
    
    if (shader)
    {
        [viewC addShaderProgram:shader sceneName:shader.name];
        [shader addTextureNamed:@"s_colorRamp" image:colorRamp];
        [shader setUniformFloatNamed:kMaplyLAZShaderPointSize val:6.0];
    }
    
    return shader;
}
