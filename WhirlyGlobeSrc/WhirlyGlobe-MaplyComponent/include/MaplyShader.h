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
    @details At this point I'm not going to fully document this.  It's complex and requires a high level of knowledge.  Look to the existing examples, consult the default shaders.
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

/** @brief Add a texture tied to the given attribute name.
    @details Shaders can have a variety of attributes passed to them.  This is incompletely implemented and documented.  In this particular case we add the given image, convert it to a texture and tie it to the shader attribute name.
    @param shaderAttrName The name of the attribute in the shader.  This should be compatible with a texture.
    @param image The UIImage we'll convert to a texture and pass in.  This UIImage will be tracked by the view controller and disposed of when we're finished iwht it.
  */
- (void)addTextureNamed:(NSString *)shaderAttrName image:(UIImage *)image;

/** @brief Check if the shader is valid.
    @details The shader setup can fail in a number of ways.  Check this after creating the shader to see if it succeded.  If not, look to getError to see why.
  */
- (bool)valid;

/** @brief Return the compilation error if there was one.
    @details Shader construction can fail in a number of interesting ways.  Call valid to see if it did fail, and then call this method to see why.
  */
- (NSString *)getError;

@end
