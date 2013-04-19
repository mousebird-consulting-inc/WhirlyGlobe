/*
 *  Lighting.mm
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

#import "Lighting.h"
#import "GLUtils.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation WhirlyKitDirectionalLight

- (id)init
{
    self = [super init];
    if (!self)
        return nil;
    
    viewDependent = true;
    
    return self;
}

- (bool)bindToProgram:(WhirlyKit::OpenGLES2Program *)program index:(int)index modelMatrix:(Eigen::Matrix4f &)modelMat
{
    char name[200];
    sprintf(name,"light[%d].viewdepend",index);
    const OpenGLESUniform *viewDependUni = program->findUniform(name);
    sprintf(name,"light[%d].direction",index);
    const OpenGLESUniform *dirUni = program->findUniform(name);
    sprintf(name,"light[%d].halfplane",index);
    const OpenGLESUniform *halfUni = program->findUniform(name);
    sprintf(name,"light[%d].ambient",index);
    const OpenGLESUniform *ambientUni = program->findUniform(name);
    sprintf(name,"light[%d].diffuse",index);
    const OpenGLESUniform *diffuseUni = program->findUniform(name);
    sprintf(name,"light[%d].specular",index);
    const OpenGLESUniform *specularUni = program->findUniform(name);
    
    Vector3f dir = pos.normalized();
    Vector3f halfPlane = (dir + Vector3f(0,0,1)).normalized();
    
    if (viewDependUni)
    {
        glUniform1f(viewDependUni->index, (viewDependent ? 0.0 : 1.0));
        CheckGLError("WhirlyKitDirectionalLight::bindToProgram glUniform1f");
    }
    if (dirUni)
    {
        glUniform3f(dirUni->index, dir.x(), dir.y(), dir.z());
        CheckGLError("WhirlyKitDirectionalLight::bindToProgram glUniform3f");
    }
    if (halfUni)
    {
        glUniform3f(halfUni->index, halfPlane.x(), halfPlane.y(), halfPlane.z());
        CheckGLError("WhirlyKitDirectionalLight::bindToProgram glUniform3f");        
    }
    if (ambientUni)
    {
        glUniform4f(ambientUni->index, ambient.x(), ambient.y(), ambient.z(), ambient.w());
        CheckGLError("WhirlyKitDirectionalLight::bindToProgram glUniform4f");
    }
    if (diffuseUni)
    {
        glUniform4f(diffuseUni->index, diffuse.x(), diffuse.y(), diffuse.z(), diffuse.w());
        CheckGLError("WhirlyKitDirectionalLight::bindToProgram glUniform4f");
    }
    if (specularUni)
    {
        glUniform4f(specularUni->index, specular.x(), specular.y(), specular.z(), specular.w());
        CheckGLError("WhirlyKitDirectionalLight::bindToProgram glUniform4f");
    }
    
    return (dirUni && halfUni && ambientUni && diffuseUni && specularUni);
}

@end

@implementation WhirlyKitMaterial

- (id)init
{
    self = [super init];
    
    ambient = Vector4f(1.0,1.0,1.0,1.0);
    diffuse = Vector4f(1.0,1.0,1.0,1.0);
//    specular = Vector4f(1.0,1.0,1.0,1.0);
//    specularExponent = 100.0;
    specular = Vector4f(0.0,0.0,0.0,0.0);
    specularExponent = 1.0;
    
    return self;
}

- (bool)bindToProgram:(WhirlyKit::OpenGLES2Program *)program
{
    return program->setUniform("material.ambient", ambient) &&
           program->setUniform("material.diffuse", diffuse) &&
           program->setUniform("material.specular", specular) &&
           program->setUniform("material.specular_exponent", specularExponent);
}


@end
