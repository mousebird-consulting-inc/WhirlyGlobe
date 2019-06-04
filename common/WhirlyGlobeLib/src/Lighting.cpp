/*
 *  Lighting.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2019 mousebird consulting.
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
#include "Program.h"

using namespace Eigen;

namespace WhirlyKit
{
    
DirectionalLight::DirectionalLight()
{
    viewDependent = true;
    pos = Eigen::Vector3f(0,0,0);
    ambient = Eigen::Vector4f(1,1,1,1);
    diffuse = Eigen::Vector4f(1,1,1,1);
    specular = Eigen::Vector4f(0,0,0,0);
}

DirectionalLight::~DirectionalLight()
{
}

bool DirectionalLight::bindToProgram(Program *program, int index, Eigen::Matrix4f modelMat) const
{
    Eigen::Vector3f dir = pos.normalized();
    Eigen::Vector3f halfPlane = (dir + Eigen::Vector3f(0,0,1)).normalized();

    program->setUniform(lightViewDependNameIDs[index], (viewDependent ? 0.0f : 1.0f));
    program->setUniform(lightDirectionNameIDs[index], dir);
    program->setUniform(lightHalfplaneNameIDs[index], halfPlane);
    program->setUniform(lightAmbientNameIDs[index], ambient);
    program->setUniform(lightDiffuseNameIDs[index], diffuse);
    program->setUniform(lightSpecularNameIDs[index], specular);

    return true;
}


Material::Material() :
    ambient(Eigen::Vector4f(1,1,1,1)),
    diffuse(Eigen::Vector4f(1,1,1,1)),
    specular(Eigen::Vector4f(1,1,1,1)),
    specularExponent(1)
{
}

Material::~Material()
{
}

bool Material::bindToProgram(Program *program)
{
    program->setUniform(materialAmbientNameID, ambient);
    program->setUniform(materialDiffuseNameID, diffuse);
    program->setUniform(materialSpecularNameID, specular);
    program->setUniform(materialSpecularExponentNameID, specularExponent);

    return true;
}

}

