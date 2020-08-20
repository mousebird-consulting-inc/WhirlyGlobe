/*
 *  VertexAttribute.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/8/19.
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

#import "VertexAttribute.h"

using namespace Eigen;

namespace WhirlyKit
{
    
VertexAttribute::VertexAttribute(BDAttributeDataType dataType,int slot,StringIdentity nameID)
: dataType(dataType), nameID(nameID), data(NULL), slot(slot)
{
    defaultData.vec3[0] = 0.0;
    defaultData.vec3[1] = 0.0;
    defaultData.vec3[2] = 0.0;
    defaultData.vec4[3] = 0.0;
}

VertexAttribute::~VertexAttribute()
{
    clear();
}

VertexAttribute::VertexAttribute(const VertexAttribute &that)
: dataType(that.dataType), nameID(that.nameID), data(NULL), defaultData(that.defaultData), slot(that.slot)
{
}

VertexAttribute VertexAttribute::templateCopy() const
{
    VertexAttribute newAttr(*this);
    return newAttr;
}

BDAttributeDataType VertexAttribute::getDataType() const
{
    return dataType;
}

void VertexAttribute::setDefaultColor(const RGBAColor &color)
{
    defaultData.color[0] = color.r;
    defaultData.color[1] = color.g;
    defaultData.color[2] = color.b;
    defaultData.color[3] = color.a;
}

void VertexAttribute::setDefaultVector2f(const Eigen::Vector2f &vec)
{
    defaultData.vec2[0] = vec.x();
    defaultData.vec2[1] = vec.y();
}

void VertexAttribute::setDefaultVector3f(const Eigen::Vector3f &vec)
{
    defaultData.vec3[0] = vec.x();
    defaultData.vec3[1] = vec.y();
    defaultData.vec3[2] = vec.z();
}

void VertexAttribute::setDefaultFloat(float val)
{
    defaultData.floatVal = val;
}

void VertexAttribute::addColor(const RGBAColor &color)
{
    if (dataType != BDChar4Type)
        return;
    
    if (!data)
        data = new std::vector<RGBAColor>();
    std::vector<RGBAColor> *colors = (std::vector<RGBAColor> *)data;
    (*colors).push_back(color);
}

void VertexAttribute::addVector2f(const Eigen::Vector2f &vec)
{
    if (dataType != BDFloat2Type)
        return;
    
    if (!data)
        data = new std::vector<Vector2f>();
    std::vector<Vector2f> *vecs = (std::vector<Vector2f> *)data;
    (*vecs).push_back(vec);
}

void VertexAttribute::addVector3f(const Eigen::Vector3f &vec)
{
    if (dataType != BDFloat3Type)
        return;
    
    if (!data)
        data = new std::vector<Vector3f>();
    std::vector<Vector3f> *vecs = (std::vector<Vector3f> *)data;
    (*vecs).push_back(vec);
}

void VertexAttribute::addVector4f(const Eigen::Vector4f &vec)
{
    if (dataType != BDFloat4Type)
        return;
    
    if (!data)
        data = new std::vector<Vector4f>();
    std::vector<Vector4f> *vecs = (std::vector<Vector4f> *)data;
    (*vecs).push_back(vec);
}

void VertexAttribute::addFloat(float val)
{
    if (dataType != BDFloatType)
        return;
    
    if (!data)
        data = new std::vector<float>();
    std::vector<float> *floats = (std::vector<float> *)data;
    (*floats).push_back(val);
}

void VertexAttribute::addInt(int val)
{
    if (dataType != BDIntType)
        return;
    
    if (!data)
        data = new std::vector<int>();
    std::vector<int> *ints = (std::vector<int> *)data;
    (*ints).push_back(val);
}

/// Reserve size in the data array
void VertexAttribute::reserve(int size)
{
    switch (dataType)
    {
        case BDFloat4Type:
        {
            if (!data)
                data = new std::vector<Vector4f>();
            std::vector<Vector4f> *vecs = (std::vector<Vector4f> *)data;
            vecs->reserve(size);
        }
            break;
        case BDFloat3Type:
        {
            if (!data)
                data = new std::vector<Vector3f>();
            std::vector<Vector3f> *vecs = (std::vector<Vector3f> *)data;
            vecs->reserve(size);
        }
            break;
        case BDFloat2Type:
        {
            if (!data)
                data = new std::vector<Vector2f>();
            std::vector<Vector2f> *vecs = (std::vector<Vector2f> *)data;
            vecs->reserve(size);
        }
            break;
        case BDChar4Type:
        {
            if (!data)
                data = new std::vector<RGBAColor>();
            std::vector<RGBAColor> *colors = (std::vector<RGBAColor> *)data;
            colors->reserve(size);
        }
            break;
        case BDFloatType:
        {
            if (!data)
                data = new std::vector<float>();
            std::vector<float> *floats = (std::vector<float> *)data;
            floats->reserve(size);
        }
            break;
        case BDIntType:
        {
            if (!data)
                data = new std::vector<int>();
            std::vector<int> *ints = (std::vector<int> *)data;
            ints->reserve(size);
        }
            break;
        case BDDataTypeMax:
            break;
    }
}

/// Number of elements in our array
int VertexAttribute::numElements() const
{
    if (!data)
        return 0;
    
    switch (dataType)
    {
        case BDFloat4Type:
        {
            std::vector<Vector4f> *vecs = (std::vector<Vector4f> *)data;
            return (int)vecs->size();
        }
            break;
        case BDFloat3Type:
        {
            std::vector<Vector3f> *vecs = (std::vector<Vector3f> *)data;
            return (int)vecs->size();
        }
            break;
        case BDFloat2Type:
        {
            std::vector<Vector2f> *vecs = (std::vector<Vector2f> *)data;
            return (int)vecs->size();
        }
            break;
        case BDChar4Type:
        {
            std::vector<RGBAColor> *colors = (std::vector<RGBAColor> *)data;
            return (int)colors->size();
        }
            break;
        case BDFloatType:
        {
            std::vector<float> *floats = (std::vector<float> *)data;
            return (int)floats->size();
        }
            break;
        case BDIntType:
        {
            std::vector<int> *ints = (std::vector<int> *)data;
            return (int)ints->size();
        }
        case BDDataTypeMax:
            return 0;
            break;
    }
}

/// Return the size of a single element
int VertexAttribute::size() const
{
    switch (dataType)
    {
        case BDFloat4Type:
            return 4*4;
            break;
        case BDFloat3Type:
            return 4*3;
            break;
        case BDFloat2Type:
            return 4*2;
            break;
        case BDChar4Type:
            return sizeof(unsigned char)*4;
            break;
        case BDFloatType:
            return 4;
            break;
        case BDIntType:
            return 4;
            break;
        case BDDataTypeMax:
            return 0;
            break;
    }
}

SingleVertexAttributeInfo::SingleVertexAttributeInfo()
: nameID(0), type(BDDataTypeMax)
{
}

SingleVertexAttributeInfo::SingleVertexAttributeInfo(StringIdentity nameID,int slot,BDAttributeDataType type)
: nameID(nameID), type(type), slot(slot)
{
}

int SingleVertexAttributeInfo::size() const
{
    switch (type)
    {
        case BDFloat4Type:
            return 4*4;
            break;
        case BDFloat3Type:
            return 4*3;
            break;
        case BDFloat2Type:
            return 4*2;
            break;
        case BDChar4Type:
            return sizeof(unsigned char)*4;
            break;
        case BDFloatType:
            return 4;
            break;
        case BDIntType:
            return 4;
            break;
        case BDDataTypeMax:
            return 0;
            break;
    }
}

SingleVertexAttribute::SingleVertexAttribute()
{
}

SingleVertexAttribute::SingleVertexAttribute(StringIdentity nameID,int slot,float floatVal)
: SingleVertexAttributeInfo(nameID,slot,BDFloatType)
{
    data.floatVal = floatVal;
}

SingleVertexAttribute::SingleVertexAttribute(StringIdentity nameID,int slot,int intVal)
: SingleVertexAttributeInfo(nameID,slot,BDIntType)
{
    data.intVal = intVal;
}

SingleVertexAttribute::SingleVertexAttribute(StringIdentity nameID,int slot,unsigned char colorVal[4])
: SingleVertexAttributeInfo(nameID,slot,BDChar4Type)
{
    for (unsigned int ii=0;ii<4;ii++)
        data.color[ii] = colorVal[ii];
}

SingleVertexAttribute::SingleVertexAttribute(StringIdentity nameID,int slot,float vec0,float vec1)
: SingleVertexAttributeInfo(nameID,slot,BDFloat2Type)
{
    data.vec2[0] = vec0;
    data.vec2[1] = vec1;
}

SingleVertexAttribute::SingleVertexAttribute(StringIdentity nameID,int slot,float vec0,float vec1,float vec2)
: SingleVertexAttributeInfo(nameID,slot,BDFloat3Type)
{
    data.vec3[0] = vec0;
    data.vec3[1] = vec1;
    data.vec3[2] = vec2;
}

SingleVertexAttribute::SingleVertexAttribute(StringIdentity nameID,int slot,float vec0,float vec1,float vec2, float vec3)
: SingleVertexAttributeInfo(nameID,slot,BDFloat4Type)
{
    data.vec4[0] = vec0;
    data.vec4[1] = vec1;
    data.vec4[2] = vec2;
    data.vec4[3] = vec3;
}

/// Clean out the data array
void VertexAttribute::clear()
{
    if (data)
    {
        switch (dataType)
        {
            case BDFloat4Type:
            {
                std::vector<Vector4f> *vecs = (std::vector<Vector4f> *)data;
                delete vecs;
            }
                break;
            case BDFloat3Type:
            {
                std::vector<Vector3f> *vecs = (std::vector<Vector3f> *)data;
                delete vecs;
            }
                break;
            case BDFloat2Type:
            {
                std::vector<Vector2f> *vecs = (std::vector<Vector2f> *)data;
                delete vecs;
            }
                break;
            case BDChar4Type:
            {
                std::vector<RGBAColor> *colors = (std::vector<RGBAColor> *)data;
                delete colors;
            }
                break;
            case BDFloatType:
            {
                std::vector<float> *floats = (std::vector<float> *)data;
                delete floats;
            }
                break;
            case BDIntType:
            {
                std::vector<int> *ints = (std::vector<int> *)data;
                delete ints;
            }
                break;
            case BDDataTypeMax:
                break;
        }
    }
    data = NULL;
}

/// Return a pointer to the given element
void *VertexAttribute::addressForElement(int which)
{
    switch (dataType)
    {
        case BDFloat4Type:
        {
            std::vector<Vector4f> *vecs = (std::vector<Vector4f> *)data;
            return &(*vecs)[which];;
        }
            break;
        case BDFloat3Type:
        {
            std::vector<Vector3f> *vecs = (std::vector<Vector3f> *)data;
            return &(*vecs)[which];;
        }
            break;
        case BDFloat2Type:
        {
            std::vector<Vector2f> *vecs = (std::vector<Vector2f> *)data;
            return &(*vecs)[which];
        }
            break;
        case BDChar4Type:
        {
            std::vector<RGBAColor> *colors = (std::vector<RGBAColor> *)data;
            return &(*colors)[which];
        }
            break;
        case BDFloatType:
        {
            std::vector<float> *floats = (std::vector<float> *)data;
            return &(*floats)[which];
        }
            break;
        case BDIntType:
        {
            std::vector<int> *ints = (std::vector<int> *)data;
            return &(*ints)[which];
        }
            break;
        case BDDataTypeMax:
            return NULL;
            break;
    }
    
    return NULL;
}

void VertexAttributeSetConvert(const SingleVertexAttributeSet &attrSet,SingleVertexAttributeInfoSet &infoSet)
{
    for (auto it : attrSet)
        infoSet.insert(it);
}

bool VertexAttributesAreCompatible(const SingleVertexAttributeInfoSet &infoSet,const SingleVertexAttributeSet &attrSet)
{
    if (infoSet.size() != attrSet.size())
        return false;
    
    auto itA = infoSet.begin();
    auto itB = attrSet.begin();
    for (;itA != infoSet.end(); ++itA, ++itB)
    {
        if (itA->nameID != itB->nameID)
            return false;
        if (itA->type != itB->type)
            return false;
    }
    
    return true;
}
    
}
