/*
 *  Lighting.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 11/6/12.
 *  Copyright 2011-2012 mousebird consulting
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
#import "WhirlyVector.h"
#import "OpenGLES2Program.h"

#define kWKOGLNumLights "u_numLights"

/** This implements a simple directional light source
  */
@interface WhirlyKitDirectionalLight : NSObject
{
@public
    /// Light position
    Eigen::Vector3f pos;
    /// If set, we won't process the light position through the model matrix
    bool viewDependent;
    /// Ambient light color
    Eigen::Vector4f ambient;
    /// Diffuse light color
    Eigen::Vector4f diffuse;
    /// Specular light color
    Eigen::Vector4f specular;
}

/// Bind this light (given the index) to the program.
/// Don't call this yourself.
- (bool)bindToProgram:(WhirlyKit::OpenGLES2Program *)program index:(int)index modelMatrix:(Eigen::Matrix4f &)modelMat;

@end

/** This is a simple material definition.
 */
@interface WhirlyKitMaterial : NSObject
{
@public
    /// Ambient material color
    Eigen::Vector4f ambient;
    /// Diffuse material color
    Eigen::Vector4f diffuse;
    /// Specular component of material color
    Eigen::Vector4f specular;
    /// Specular exponent used in lighting
    float specularExponent;
}

/// Bind this material to a the given OpenGL ES program.
/// Don't call this yourself.
- (bool)bindToProgram:(WhirlyKit::OpenGLES2Program *)program;

@end
