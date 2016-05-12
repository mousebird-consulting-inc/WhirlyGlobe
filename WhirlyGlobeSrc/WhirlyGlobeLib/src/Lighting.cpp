/*
 *  Lighting.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2016 mousebird consulting. All rights reserved.
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
#include "Lighting.h"
#include "GLUtils.h"
#include "OpenGLES2Program.h"

using namespace Eigen;

namespace WhirlyKit
{

WhirlyKitDirectionalLight::WhirlyKitDirectionalLight()
{
    viewDependent = true;
    pos = Eigen::Vector3f(0,0,0);
    ambient = Eigen::Vector4f(1,1,1,1);
    diffuse = Eigen::Vector4f(1,1,1,1);
    specular = Eigen::Vector4f(0,0,0,0);
};

WhirlyKitDirectionalLight::~WhirlyKitDirectionalLight()
{
}

bool WhirlyKitDirectionalLight::bindToProgram(OpenGLES2Program *program, int index, Eigen::Matrix4f modelMat) const
    {
    char name[128];
    sprintf(name, "light[%d].viewdepend", index);
    const OpenGLESUniform *viewDependUni = program->findUniform(name);
    sprintf(name, "light[%d].direction", index);
    const OpenGLESUniform *dirUni = program->findUniform(name);
    sprintf(name, "light[%d].halfplane", index);
    const OpenGLESUniform *halfUni = program->findUniform(name);
    sprintf(name, "light[%d].ambient", index);
    const OpenGLESUniform *ambientUni = program->findUniform(name);
    sprintf(name, "light[%d].diffuse", index);
    const OpenGLESUniform *diffuseUni = program->findUniform(name);
    sprintf(name, "light[%d].specular", index);
    const OpenGLESUniform *specularUni = program->findUniform(name);

    Eigen::Vector3f dir = pos.normalized();
    Eigen::Vector3f halfPlane = (dir + Eigen::Vector3f(0,0,1)).normalized();

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


WhirlyKitMaterial::WhirlyKitMaterial() :
    ambient(Eigen::Vector4f(1,1,1,1)),
    diffuse(Eigen::Vector4f(1,1,1,1)),
    specular(Eigen::Vector4f(1,1,1,1)),
    specularExponent(1)
{
}

WhirlyKitMaterial::~WhirlyKitMaterial()
{
}

bool WhirlyKitMaterial::bindToProgram(OpenGLES2Program *program)
{
    return program->setUniform("material.ambient", ambient) &&
           program->setUniform("material.diffuse", diffuse) &&
           program->setUniform("material.specular", specular) &&
           program->setUniform("material.specular_exponent", specularExponent);
}

}

