/*
 *  MaplyTriangleShaders_private.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 8/21/18.
 *  Copyright 2011-2018 mousebird consulting
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

#import <UIKit/UIKit.h>
#import "MaplyShader.h"
#import <WhirlyGlobe.h>

namespace WhirlyKit
{
 
// Triangle shader with lighting
OpenGLES2Program *BuildDefaultTriShaderLighting(const std::string &name);
// Triangle shader without lighting
OpenGLES2Program *BuildDefaultTriShaderNoLighting(const std::string &name);
// Triangle shader for models
OpenGLES2Program *BuildDefaultTriShaderModel(const std::string &name);
// Triangles with screen textures
OpenGLES2Program *BuildDefaultTriShaderScreenTexture(const std::string &name);
// Triangles with multiple textures
OpenGLES2Program *BuildDefaultTriShaderMultitex(const std::string &name);
// Triangles that use the ramp textures
OpenGLES2Program *BuildDefaultTriShaderRamptex(const std::string &name);
// Day/night support for triangles
OpenGLES2Program *BuildDefaultTriShaderNightDay(const std::string &name);

}
