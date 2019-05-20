/*
 *  TriangleShaders.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 8/21/18.
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

#import "ProgramGLES.h"
#import "SceneRenderer.h"

namespace WhirlyKit
{
 
// Triangle shader with lighting
ProgramGLES *BuildDefaultTriShaderLightingGLES(const std::string &name,SceneRenderer *renderer);
// Triangle shader without lighting
ProgramGLES *BuildDefaultTriShaderNoLightingGLES(const std::string &name,SceneRenderer *renderer);
// Triangle shader for models
ProgramGLES *BuildDefaultTriShaderModelGLES(const std::string &name,SceneRenderer *renderer);
// Triangles with screen textures
ProgramGLES *BuildDefaultTriShaderScreenTextureGLES(const std::string &name,SceneRenderer *renderer);
// Triangles with multiple textures
ProgramGLES *BuildDefaultTriShaderMultitexGLES(const std::string &name,SceneRenderer *renderer);
// Triangles that use the ramp textures
ProgramGLES *BuildDefaultTriShaderRamptexGLES(const std::string &name,SceneRenderer *renderer);
// Day/night support for triangles
ProgramGLES *BuildDefaultTriShaderNightDayGLES(const std::string &name,SceneRenderer *renderer);

}
