/*  VertexAttribute.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/8/19.
 *  Copyright 2011-2023 mousebird consulting
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

#import "VertexAttribute.h"
#import "WhirlyKitLog.h"

using namespace Eigen;

namespace WhirlyKit
{

VertexAttribute::VertexAttribute(BDAttributeDataType dataType,int slot,StringIdentity nameID) :
    dataType(dataType),
    nameID(nameID),
    slot(slot)
{
}

VertexAttribute::VertexAttribute(const VertexAttribute &that) :
    dataType(that.dataType),
    nameID(that.nameID),
    defaultData(that.defaultData),
    slot(that.slot)
{
}

VertexAttribute::~VertexAttribute()
{
    try
    {
        clear();
    }
    WK_STD_DTOR_CATCH()
}

VertexAttribute VertexAttribute::templateCopy() const
{
    return *this;
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

namespace
{
    template<typename T, typename A>
    static std::vector<T,A> *make(void *p) { return p ? (std::vector<T,A> *)p : new std::vector<T,A>(); }
    template <typename T, typename A>
    static std::vector<T,A> *op(void *p, std::function<void(std::vector<T,A> *)> f) {
        auto *vec = make<T,A>(p);
        f(vec);
        return vec;
    }
    template<typename T, typename A = std::allocator<T>>
    static std::vector<T,A> *add(void *p, const T &v) {
        return op<T,A>(p, [&](std::vector<T,A> *vec){ vec->push_back(v); });
    }
    template<typename T, typename A = std::allocator<T>>
    static std::vector<T,A> *reserve(void *p, int size) { return op<T,A>(p, [=](std::vector<T,A> *vec){ vec->reserve(size); }); }
    template<typename T, typename A = Eigen::aligned_allocator<T>>
    static std::vector<T,A> *reserveEigen(void *p, int size) { return reserve<T,A>(p, size); }
    template<typename T, typename A = std::allocator<T>>
    static T *addr(void *p, int n) { return &((std::vector<T,A> *)p)->operator[](n); }
    template<typename T, typename A = Eigen::aligned_allocator<T>>
    static T *addrEigen(void *p, int n) { return &((std::vector<T,A> *)p)->operator[](n); }
}

void VertexAttribute::addColor(const RGBAColor &color)
{
    if (dataType == BDChar4Type)
    {
        data = add(data, color);
    }
}

void VertexAttribute::addVector2f(const Eigen::Vector2f &vec)
{
    if (dataType == BDFloat2Type)
    {
        data = add<Eigen::Vector2f,Eigen::aligned_allocator<Eigen::Vector2f>>(data, vec);
    }
}

void VertexAttribute::addVector3f(const Eigen::Vector3f &vec)
{
    if (dataType == BDFloat3Type)
    {
        data = add<Vector3f,Eigen::aligned_allocator<Vector3f>>(data, vec);
    }
}

void VertexAttribute::addVector4f(const Eigen::Vector4f &vec)
{
    if (dataType == BDFloat4Type)
    {
        data = add<Vector4f,Eigen::aligned_allocator<Vector4f>>(data, vec);
    }
}

void VertexAttribute::addFloat(float val)
{
    if (dataType == BDFloatType)
    {
        data = add(data, val);
    }
}

void VertexAttribute::addInt(int val)
{
    if (dataType == BDIntType)
    {
        data = add(data, val);
    }
}

void VertexAttribute::addInt64(int64_t val)
{
    if (dataType == BDInt64Type)
    {
        data = add(data, val);
    }
}

/// Reserve size in the data array
void VertexAttribute::reserve(int size)
{
    switch (dataType)
    {
        case BDFloat4Type: data = WhirlyKit::reserveEigen<Vector4f>(data, size); break;
        case BDFloat3Type: data = WhirlyKit::reserveEigen<Vector3f>(data, size); break;
        case BDFloat2Type: data = WhirlyKit::reserveEigen<Vector2f>(data, size); break;
        case BDChar4Type:  data = WhirlyKit::reserve<RGBAColor>(data, size); break;
        case BDFloatType:  data = WhirlyKit::reserve<float>(data, size); break;
        case BDIntType:    data = WhirlyKit::reserve<int>(data, size); break;
        case BDInt64Type:  data = WhirlyKit::reserve<int64_t>(data, size); break;
        default:
        case BDDataTypeMax:
            break;
    }
}

/// Number of elements in our array
int VertexAttribute::numElements() const
{
    switch (data ? dataType : BDDataTypeMax)
    {
        case BDFloat4Type: return (int)((const std::vector<Vector4f, Eigen::aligned_allocator<Vector4f>> *)data)->size();
        case BDFloat3Type: return (int)((const std::vector<Vector3f, Eigen::aligned_allocator<Vector3f>> *)data)->size();
        case BDFloat2Type: return (int)((const std::vector<Vector2f, Eigen::aligned_allocator<Vector2f>> *)data)->size();
        case BDChar4Type:  return (int)((const std::vector<RGBAColor> *)data)->size();
        case BDFloatType:  return (int)((const std::vector<float> *)data)->size();
        case BDIntType:    return (int)((const std::vector<int> *)data)->size();
        case BDInt64Type:  return (int)((const std::vector<int64_t> *)data)->size();
        default:
        case BDDataTypeMax:
            return 0;
    }
}

/// Return the size of a single element
int VertexAttribute::size() const
{
    switch (dataType)
    {
        case BDFloat4Type:  return 4*4;
        case BDFloat3Type:  return 4*3;
        case BDFloat2Type:  return 4*2;
        case BDChar4Type:   return sizeof(unsigned char)*4;
        case BDFloatType:
        case BDIntType:     return 4;
        case BDInt64Type:   return 8;
        default:
        case BDDataTypeMax: return 0;
    }
}

SingleVertexAttributeInfo::SingleVertexAttributeInfo(StringIdentity nameID,int slot,BDAttributeDataType type) :
    type(type),
    slot(slot),
    nameID(nameID)
{
}

int SingleVertexAttributeInfo::size() const
{
    switch (type)
    {
        case BDFloat4Type:  return 4*4;
        case BDFloat3Type:  return 4*3;
        case BDFloat2Type:  return 4*2;
        case BDChar4Type:   return sizeof(unsigned char)*4;
        case BDFloatType:
        case BDIntType:     return 4;
        case BDInt64Type:   return 8;
        case BDDataTypeMax: return 0;
    }
}

SingleVertexAttribute::SingleVertexAttribute(StringIdentity nameID,int slot,float floatVal) :
    SingleVertexAttributeInfo(nameID,slot,BDFloatType)
{
    data.floatVal = floatVal;
}

SingleVertexAttribute::SingleVertexAttribute(StringIdentity nameID,int slot,int intVal) :
    SingleVertexAttributeInfo(nameID,slot,BDIntType)
{
    data.intVal = intVal;
}

SingleVertexAttribute::SingleVertexAttribute(StringIdentity nameID,int slot,int64_t intVal)
: SingleVertexAttributeInfo(nameID,slot,BDInt64Type)
{
    data.int64Val = intVal;
}

SingleVertexAttribute::SingleVertexAttribute(StringIdentity nameID,int slot,const unsigned char colorVal[4]) :
    SingleVertexAttributeInfo(nameID,slot,BDChar4Type)
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
    switch (data ? dataType : BDDataTypeMax)
    {
        case BDFloat4Type: delete (std::vector<Vector4f, Eigen::aligned_allocator<Vector4f>> *)data; break;
        case BDFloat3Type: delete (std::vector<Vector3f, Eigen::aligned_allocator<Vector3f>> *)data; break;
        case BDFloat2Type: delete (std::vector<Vector2f, Eigen::aligned_allocator<Vector2f>> *)data; break;
        case BDChar4Type:  delete (std::vector<RGBAColor> *)data; break;
        case BDFloatType:  delete (std::vector<float> *)data;     break;
        case BDIntType:    delete (std::vector<int> *)data;       break;
        case BDInt64Type:  delete (std::vector<int64_t> *)data;   break;
        default:
        case BDDataTypeMax: break;
    }
    data = nullptr;
}

/// Return a pointer to the given element
void *VertexAttribute::addressForElement(int which) const
{
    switch (dataType)
    {
        case BDFloat4Type: return WhirlyKit::addrEigen<Vector4f>(data, which);
        case BDFloat3Type: return WhirlyKit::addrEigen<Vector3f>(data, which);
        case BDFloat2Type: return WhirlyKit::addrEigen<Vector2f>(data, which);
        case BDChar4Type: return WhirlyKit::addr<RGBAColor>(data, which);
        case BDFloatType: return WhirlyKit::addr<float>(data, which);
        case BDIntType: return WhirlyKit::addr<int>(data, which);
        case BDInt64Type: return WhirlyKit::addr<int64_t>(data, which);
        default:
        case BDDataTypeMax:
            return nullptr;
    }
}

void VertexAttributeSetConvert(const SingleVertexAttributeSet &attrSet,SingleVertexAttributeInfoSet &infoSet)
{
    infoSet.insert(attrSet.begin(), attrSet.end());
}

bool VertexAttributesAreCompatible(const SingleVertexAttributeInfoSet &infoSet,const SingleVertexAttributeSet &attrSet)
{
    if (infoSet.size() != attrSet.size())
        return false;
    
    auto itA = infoSet.begin();
    auto itB = attrSet.begin();
    for (;itA != infoSet.end(); ++itA, ++itB)
    {
        if (itA->nameID != itB->nameID || itA->type != itB->type)
            return false;
    }
    
    return true;
}
    
}
