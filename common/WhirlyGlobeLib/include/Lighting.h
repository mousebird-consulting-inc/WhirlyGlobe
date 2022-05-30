/*  Lighting.h
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2022 mousebird consulting.
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
 */

#import "Platform.h"
#import "WhirlyVector.h"

#define kWKOGLNumLights "u_numLights"

namespace WhirlyKit {

/** This implements a simple directional light source
*/
struct DirectionalLight
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    DirectionalLight() = default;
    virtual ~DirectionalLight() = default;

    /// If set, we won't process the light position through the model matrix
    bool getViewDependent() const { return viewDependent; }
    void setViewDependent(bool value) { viewDependent = value; }

    /// Light position
    const Eigen::Vector3f& getPos() const { return pos; }
    void setPos(const Eigen::Vector3f& value){ pos = value; }

    /// Ambient light color
    const Eigen::Vector4f& getAmbient() const { return ambient; }
    void setAmbient(const Eigen::Vector4f& value){ ambient = value; }

    /// Diffuse light color
    const Eigen::Vector4f& getDiffuse() const { return diffuse; }
    void setDiffuse(const Eigen::Vector4f& value){ diffuse = value; }

    /// Specular light color
    const Eigen::Vector4f getSpecular() const { return specular; }
    void setSpecular(const Eigen::Vector4f& value){ specular = value; }

    bool operator==(const DirectionalLight &that) const
    {
        return viewDependent == that.viewDependent &&
            pos == that.pos &&
            ambient == that.ambient &&
            diffuse == that.diffuse &&
            specular == that.specular;
    }

protected:
    Eigen::Vector4f ambient = { 1.0f, 1.0f, 1.0f, 1.0f };
    Eigen::Vector4f diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
    Eigen::Vector4f specular = { 0.0f, 0.0f, 0.0f, 0.0f };
    Eigen::Vector3f pos = { 0.0f, 0.0f, 0.0f };
    bool viewDependent = true;
};


/** This is a simple material definition.
 */
struct Material
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    Material() = default;
    virtual ~Material() = default;

    /// Ambient material color
    void setAmbient(const Eigen::Vector4f& value) { ambient = value; }
    const Eigen::Vector4f& getAmbient() const { return ambient; }

    /// Diffuse material color
    void setDiffuse(const Eigen::Vector4f& value) { diffuse = value; }
    const Eigen::Vector4f& getDiffuse() const { return diffuse; }

    /// Specular component of material color
    void setSpecular(const Eigen::Vector4f& value) { specular = value; }
    const Eigen::Vector4f& getSpecular() const { return specular; }

    /// Specular exponent used in lighting
    void setSpecularExponent(float value){ specularExponent = value; }
    float getSpecularExponent() const { return specularExponent; }

protected:
    Eigen::Vector4f ambient = { 1.0f, 1.0f, 1.0f, 1.0f };
    Eigen::Vector4f diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
    Eigen::Vector4f specular = { 1.0f, 1.0f, 1.0f, 1.0f };
    float specularExponent = 1.0f;
};

}
