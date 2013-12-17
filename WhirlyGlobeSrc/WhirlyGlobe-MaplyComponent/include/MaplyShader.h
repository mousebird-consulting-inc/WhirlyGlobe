/*
 *  MaplyShader.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/7/13.
 *  Copyright 2011-2013 mousebird consulting
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

@class MaplyBaseViewController;

/** @brief The shader is a direct interface to OpenGL ES 2.0 shader language.
    @details You can set your own shader programs in the toolkit!  Yeah, that's as complex as it sounds.
    @details The underyling toolkit makes a distinction between the name of the shader and the scene name.  The scene name is used as a way to replace the default shaders we use for triangles and lines.  This would let you replace the shaders you're already using with your own.  See the addShaderProgram:sceneName: method in the MaplyBaseViewController.
    @details You can also add your own shader and hook it up to any features that can call out a specific shader, such as the MaplyQuadImageTilesLayer.
    @details When writing a new shader, go take a look at DefaultShaderPrograms.mm, particularly the vertexShaderTri and fragmentShaderTri.  The documentation here is for the uniforms, and attributes the system is going to hook up for you.  All of these are optional, but obviously nothing much will happen if you don't use the vertices.  
 
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

/** @brief Initialize with the file names for the shader program.
    @details See initWithName:vertex:fragment:viewC: for more details on how this works.
    @param name The name of the shader program.  Used for identification and sometimes lookup.
    @param vertexFileName The file (in the bundle) containing the vertex shader.
    @param fragFileName The file (in the bundle) containing the fragment shader.
    @param baseViewC The view controller where we'll register the new shader.
    @return Returns a shader program if it succeeded.  It may not work, however, so call valid first.
  */
- (id)initWithName:(NSString *)name vertexFile:(NSString *)vertexFileName fragmentFile:(NSString *)fragFileName viewC:(MaplyBaseViewController *)baseViewC;

/** @brief Initialize with the shader programs tied to a particular view controller.
    @details This initializer will parse the given shader program, link it and return a MaplyShader if it succeeded.  It will tie that shader in to the given view controller (and really, it's renderer).  You can only use that shader in that view controller.
    @param name The name of the shader program.  Used for identification and sometimes lookup.
    @param vertexProg The string containing the full vertex program.
    @param fragProg The string containing the full fragment program.
    @param baseViewC The view controller where we'll register the new shader.
    @return Returns a shader program if it succeeded.  IT may not work, however, so call valid first.
 */
- (id)initWithName:(NSString *)name vertex:(NSString *)vertexProg fragment:(NSString *)fragProg viewC:(MaplyBaseViewController *)baseViewC;

/** @brief Name of the shader program.
    @details This is the name passed in to the init call.  You can search by this name in some circumstances.
  */
@property NSString *name;

// Note: Porting
///** @brief Add a texture tied to the given attribute name.
//    @details Shaders can have a variety of attributes passed to them.  This is incompletely implemented and documented.  In this particular case we add the given image, convert it to a texture and tie it to the shader attribute name.
//    @param shaderAttrName The name of the attribute in the shader.  This should be compatible with a texture.
//    @param image The UIImage we'll convert to a texture and pass in.  This UIImage will be tracked by the view controller and disposed of when we're finished iwht it.
//  */
//- (void)addTextureNamed:(NSString *)shaderAttrName image:(UIImage *)image;

/** @brief Set a float uniform in the shader with the given name.
    @return Returns true if there was such a uniform, false otherwise.
  */
- (bool)setUniformFloatNamed:(NSString *)uniName val:(float)val;

/** @brief Set an integer uniform in the shader with the given name.
    @return Returns true if there was such a uniform, false otherwise.
 */
- (bool)setUniformIntNamed:(NSString *)uniName val:(int)val;

/** @brief Set a 2 component float uniform in the shader with the given name.
    @return Returns true if there was such a uniform, false otherwise.
 */
- (bool)setUniformVector2Named:(NSString *)uniName x:(float)x y:(float)y;

/** @brief Set a 3 component float uniform in the shader with the given name.
    @return Returns true if there was such a uniform, false otherwise.
 */
- (bool)setUniformVector3Named:(NSString *)uniName x:(float)x y:(float)y z:(float)z;

/** @brief Set a 4 component float uniform in the shader with the given name.
    @return Returns true if there was such a uniform, false otherwise.
 */
- (bool)setUniformVector4Named:(NSString *)uniName x:(float)x y:(float)y z:(float)z w:(float)w;

/** @brief Check if the shader is valid.
    @details The shader setup can fail in a number of ways.  Check this after creating the shader to see if it succeded.  If not, look to getError to see why.
  */
- (bool)valid;

/** @brief Return the compilation error if there was one.
    @details Shader construction can fail in a number of interesting ways.  Call valid to see if it did fail, and then call this method to see why.
  */
- (NSString *)getError;

@end
