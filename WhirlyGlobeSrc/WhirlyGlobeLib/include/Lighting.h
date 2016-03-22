/*
 *  Lighting.h
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
#include <Eigen/Eigen>

#define kWKOGLNumLights "u_numLights"

namespace WhirlyKit {

class OpenGLES2Program;

/** This implements a simple directional light source
*/
class WhirlyKitDirectionalLight
{
public:
    WhirlyKitDirectionalLight();
    virtual ~WhirlyKitDirectionalLight();

    /// If set, we won't process the light position through the model matrix
    bool getViewDependent() { return viewDependent; }
    void setViewDependent(bool value) { viewDependent = value; }

    /// Light position
    const Eigen::Vector3f& getPos(){ return pos; }
    void setPos(const Eigen::Vector3f& value){ pos = value; }

    /// Ambient light color
    const Eigen::Vector4f& getAmbient() { return ambient; }
    void setAmbient(const Eigen::Vector4f& value){ ambient = value; }

    /// Diffuse light color
    const Eigen::Vector4f& getDiffuse() { return diffuse; }
    void setDiffuse(const Eigen::Vector4f& value){ diffuse = value; }

    /// Specular light color
    const Eigen::Vector4f getSpecular() { return specular; }
    void setSpecular(const Eigen::Vector4f& value){ specular = value; }

    /// Bind this light (given the index) to the program.
    /// Don't call this yourself.
    bool bindToProgram(OpenGLES2Program *program, int index, Eigen::Matrix4f modelMat);

private:
    Eigen::Vector3f pos;
    bool viewDependent;
    Eigen::Vector4f ambient;
    Eigen::Vector4f diffuse;
    Eigen::Vector4f specular;
};


/** This is a simple material definition.
 */
class WhirlyKitMaterial
{
public:
    WhirlyKitMaterial();
    virtual ~WhirlyKitMaterial();

    /// Ambient material color
    void setAmbient(const Eigen::Vector4f& value){ ambient = value; }
    const Eigen::Vector4f& getAmbient() { return ambient; }

    /// Diffuse material color
    void setDiffuse(const Eigen::Vector4f& value) { diffuse = value; }
    const Eigen::Vector4f& getDiffuse() { return diffuse; }

    /// Specular component of material color
    void setSpecular(const Eigen::Vector4f& value) { specular = value; }
    const Eigen::Vector4f& getSpecular() { return specular; }

    /// Specular exponent used in lighting
    void setSpecularExponent(float value){ specularExponent = value; }
    float getSpecularExponent() { return specularExponent; }

    /// Bind this material to a the given OpenGL ES program.
    /// Don't call this yourself.
    bool bindToProgram(OpenGLES2Program *program);

private:
    Eigen::Vector4f ambient;
    Eigen::Vector4f diffuse;
    Eigen::Vector4f specular;
    float specularExponent;
};

}
