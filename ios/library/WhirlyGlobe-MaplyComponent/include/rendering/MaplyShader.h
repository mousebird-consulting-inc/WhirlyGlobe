/*
 *  MaplyShader.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/7/13.
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

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

@protocol MaplyRenderControllerProtocol;
@class MaplyTexture;

/** 
    The various types of attributes that can be passed in to shaders.
  */
typedef NS_ENUM(NSInteger, MaplyShaderAttrType){
	MaplyShaderAttrTypeInt,
	MaplyShaderAttrTypeFloat,
	MaplyShaderAttrTypeFloat2,
	MaplyShaderAttrTypeFloat3,
	MaplyShaderAttrTypeFloat4
};

/** 
    The shader is a direct interface to OpenGL ES 2.0 shader language.
    
    You can set your own shader programs in the toolkit!  Yeah, that's as complex as it sounds.
    
    The underyling toolkit makes a distinction between the name of the shader and the scene name.  The scene name is used as a way to replace the default shaders we use for triangles and lines.  This would let you replace the shaders you're already using with your own.  See the addShaderProgram: method in the MaplyBaseViewController.
    
    You can also add your own shader and hook it up to any features that can call out a specific shader, such as the MaplyQuadImageTilesLayer.
    
    When writing a new shader, go take a look at DefaultShaderPrograms.mm, particularly the vertexShaderTri and fragmentShaderTri.  The documentation here is for the uniforms and attributes the system is going to hook up for you.  All of these are optional, but obviously nothing much will happen if you don't use the vertices.
 
**Uniform Values**
 
These are uniform values provided to each shader, if requested.
 
 |uniform|type|description|
 |:------|:---|:----------|
 |u_mvpMatrix|mat4| The model/view/projection matrix.  Shaders typically run vertices through this. |
 |u_mvMatrix|mat4| The model/view matrix.  Less comonly used. |
 |u_mvNormalMatrix|mat4| The model/view matrix for normals.  A shader typically uses this when we want view dependent lighting. |
 |u_fade|float| Available in regular drawables, but not yet in big drawables (e.g. atlases).  This is intended to fade geometry in and out over time. |
 |u_interp|float| If we're doing multiple textures, this is how to interpolate them. |
 |u_numLights|int| The number of active lights to use. |
 |light[8]|directional_light| A data structure for each active light.  See the table below. |
 |material|material_properties| Material information used to calculate lighting.  See the table below. |
 |u_hasTexture|bool| True if there's a texture available to fetch data from. |
 |s_baseMapX|sampler2D| s_baseMap0, s_baseMap1 and so forth are references to texture data. |

**Material properties**
 
These are the fields for the material properties.
 
|field|type|description|
|:----|:---|:----------|
|ambient|vec4| The ambient value for the material.  Shaders typically multiply by this value when calculating ambient lighting. |
|diffuse|vec4| The diffuse value for the material.  Shaders typically multiply by this value when calculating diffuse lighting. |
|specular|vec4| Not currently used. |
|specular_exponent|float| Not currently used. |
 
**Light properties**

These are the fields for each individual light.
 
 |field|type|description|
 |:----|:---|:----------|
 |direction|vec3| The light's direction, used in diffuse lighting. |
 |halfplane|vec3| This would be used in specular lighting. |
 |ambient|vec4| The ambient value of the light. |
 |diffuse|vec4| The diffuse value of the light. |
 |specular|vec4| Not currently used. |
 |viewdepend|float| If greater than 0.0, the shader should run each normal through the u_mvNormalMatrix. |
 
**Attributes**
 
These are the per vertex attributes provided to each vertex shader.

 |field|type|description|
 |:----|:---|:----------|
 |a_position|vec3| The position in display space for a vertex.  Shaders typically multiply this by u_mvpMatrix. |
 |a_texCoord0|vec2| If textures are present, this is the texture coordinate for the first one. |
 |a_texCoord1|vec2| If two textures are present, this is the texture coordinate for the second. |
 |a_color|vec4| An RGBA color for the vertex. |
 |a_normal|vec3| A normal in display space.  This is used purely for lighting and often run through u_mvNormalMatrix. |
 |a_elev|float| An optional elevation value provided by the MaplyQuadImageTiles layer.  You can use it to do elevation dependent shading. |
 
  */
@interface MaplyShader : NSObject

/**
    Initialize with Metal shader functions tied to a particular view controller.  Metal only.
 
    This initializer just ties the given functions to this MaplyShader.  All the real work is
    done by Metal.
 
     @param name The name of the shader program.  Used for identification and sometimes lookup.
 
     @param vertexFunc The MTLFunction for vertex processing.
 
     @param fragFunc The MTLFunction for fragment processing.

     @param baseViewC The view controller where we'll register the new shader.
 
     @return Returns a shader program if it succeeded.  IT may not work, however, so call valid first.
  */
- (nullable instancetype)initMetalWithName:(NSString *__nonnull)name vertex:(id<MTLFunction> __nonnull)vertexFunc fragment:(id<MTLFunction> __nullable)fragFunc viewC:(NSObject<MaplyRenderControllerProtocol> *__nonnull)baseViewC;

/** 
    Name of the shader program.
    
    This is the name passed in to the init call.  You can search by this name in some circumstances.
  */
@property (nonatomic,strong) NSString * __nullable name;

/**
   Present a texture to this shader for use.  Metal Only.
 
   For a Metal shader we can pass in zero or more textures starting at WKSTextureEntryLookup (DefaultShadersMTL.h).
   This index is offset from there.  Start at 0.
  */
- (void)setTexture:(MaplyTexture * __nonnull)tex forIndex:(int)idx viewC:(NSObject<MaplyRenderControllerProtocol> * __nonnull)viewC;

/**
 Remove a texture we presented to the Shader ealier.  Metal Only.
 
 The texture itself will not be deleted, just the reference to it in the shader.
 */
- (void)removeTexture:(MaplyTexture * __nonnull)tex viewC:(NSObject<MaplyRenderControllerProtocol> * __nonnull)viewC;

/**
    For Metal shaders we don't set the individual uniform values passed in, we set them all together as a block.  These are typically set
    through the ComponentObject interface, but can be set gobally here.
 
    Metal Only.
 */
- (bool)setUniformBlock:(NSData *__nonnull)uniBlock buffer:(int)bufferID;

/**
   If set, this program is expecting to be called once for each level of a render target's texture.
 Essentially, it runs a reduce operation, starting from some source and working its way up to the 1x1 texture at the top.
 */
- (void)setReduceMode:(bool)reduceMode;

/** 
    Check if the shader is valid.
    
    The shader setup can fail in a number of ways.  Check this after creating the shader to see if it succeded.  If not, look to getError to see why.
  */
- (bool)valid;

/** 
    Return the compilation error if there was one.
    
    Shader construction can fail in a number of interesting ways.  Call valid to see if it did fail, and then call this method to see why.
  */
- (NSString *__nullable)getError;

@end
