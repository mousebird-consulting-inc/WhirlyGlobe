/*
 *  Lighting.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 11/6/12.
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
    
    _viewDependent = true;
    
    return self;
}

- (void)setPos:(Eigen::Vector3f)pos
{
    _pos = pos;
}

- (void)setAmbient:(Eigen::Vector4f)ambient
{
    _ambient = ambient;
}

- (void)setDiffuse:(Eigen::Vector4f)diffuse
{
    _diffuse = diffuse;
}

- (void)setSpecular:(Eigen::Vector4f)specular
{
    _specular = specular;
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
    
    Vector3f dir = _pos.normalized();
    Vector3f halfPlane = (dir + Vector3f(0,0,1)).normalized();
    
    if (viewDependUni)
    {
        glUniform1f(viewDependUni->index, (_viewDependent ? 0.0 : 1.0));
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
        glUniform4f(ambientUni->index, _ambient.x(), _ambient.y(), _ambient.z(), _ambient.w());
        CheckGLError("WhirlyKitDirectionalLight::bindToProgram glUniform4f");
    }
    if (diffuseUni)
    {
        glUniform4f(diffuseUni->index, _diffuse.x(), _diffuse.y(), _diffuse.z(), _diffuse.w());
        CheckGLError("WhirlyKitDirectionalLight::bindToProgram glUniform4f");
    }
    if (specularUni)
    {
        glUniform4f(specularUni->index, _specular.x(), _specular.y(), _specular.z(), _specular.w());
        CheckGLError("WhirlyKitDirectionalLight::bindToProgram glUniform4f");
    }
    
    return (dirUni && halfUni && ambientUni && diffuseUni && specularUni);
}

@end

@implementation WhirlyKitMaterial

- (id)init
{
    self = [super init];
    
    _ambient = Vector4f(1.0,1.0,1.0,1.0);
    _diffuse = Vector4f(1.0,1.0,1.0,1.0);
//    specular = Vector4f(1.0,1.0,1.0,1.0);
//    specularExponent = 100.0;
    _specular = Vector4f(0.0,0.0,0.0,0.0);
    _specularExponent = 1.0;
    
    return self;
}

- (void)setAmbient:(Eigen::Vector4f)ambient
{
    _ambient = ambient;
}

- (void)setDiffuse:(Eigen::Vector4f)diffuse
{
    _diffuse = diffuse;
}

- (void)setSpecular:(Eigen::Vector4f)specular
{
    _specular = specular;
}

- (bool)bindToProgram:(WhirlyKit::OpenGLES2Program *)program
{
    return program->setUniform("material.ambient", _ambient) &&
           program->setUniform("material.diffuse", _diffuse) &&
           program->setUniform("material.specular", _specular) &&
           program->setUniform("material.specular_exponent", _specularExponent);
}


@end
