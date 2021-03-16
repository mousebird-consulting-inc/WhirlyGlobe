/*
 *  VertexAttribute.h
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

#import <vector>
#import <set>
#import <map>
#import "RawData.h"
#import "Identifiable.h"
#import "StringIndexer.h"
#import "WhirlyVector.h"
#import "ChangeRequest.h"

namespace WhirlyKit
{
/// Data types we'll accept for attributes
typedef enum {
    BDFloat4Type = 0,
    BDFloat3Type = 1,
    BDChar4Type  = 2,
    BDFloat2Type = 3,
    BDFloatType  = 4,
    BDIntType    = 5,
    BDInt64Type = 6,
    BDDataTypeMax
} BDAttributeDataType;

/// Used to keep track of attributes (other than points)
class VertexAttribute
{
public:
    VertexAttribute(BDAttributeDataType dataType,int slot,StringIdentity nameID);
    VertexAttribute(const VertexAttribute &that);
    virtual ~VertexAttribute();
    
    /// Make a copy of everything for the data
    VertexAttribute templateCopy() const;
    
    /// Return the data type
    BDAttributeDataType getDataType() const;
    
    /// Set the default color (if the type matches)
    void setDefaultColor(const RGBAColor &color);
    /// Set the default 2D vector (if the type matches)
    void setDefaultVector2f(const Eigen::Vector2f &vec);
    /// Set the default 3D vector (if the type matches)
    void setDefaultVector3f(const Eigen::Vector3f &vec);
    /// Set the default float (if the type matches)
    void setDefaultFloat(float val);
    
    /// Convenience routine to add a color (if the type matches)
    void addColor(const RGBAColor &color);
    /// Convenience routine to add a 2D vector (if the type matches)
    void addVector2f(const Eigen::Vector2f &vec);
    /// Convenience routine to add a 3D vector (if the type matches)
    void addVector3f(const Eigen::Vector3f &vec);
    /// Convenience routine to add a 4D vector (if the type matches)
    void addVector4f(const Eigen::Vector4f &vec);
    /// Convenience routine to add a float (if the type matches)
    void addFloat(float val);
    /// Convenience routine to add an int (if the type matches)
    void addInt(int val);
    /// Convenience routine to add an int64 (if the type matches)
    void addInt64(int64_t val);
    
    /// Reserve size in the data array
    void reserve(int size);
    
    /// Number of elements in our array
    int numElements() const;
    
    /// Return the size of a single element
    int size() const;
    
    /// Clean out the data array
    void clear();
    
    /// Return a pointer to the given element
    void *addressForElement(int which);
    
public:
    /// Data type for the attribute data
    BDAttributeDataType dataType;
    /// Name used in the shader
    StringIdentity nameID;
    /// Default value to pass to renderer if there's no data array
    union {
        float vec4[4];
        float vec3[3];
        float vec2[2];
        float floatVal;
        unsigned char color[4];
        int intVal;
    } defaultData;
    /// Used by Metal instead of a name
    int slot;
    /// std::vector of attribute data.  Type is known by the caller.
    void *data;
};
typedef std::shared_ptr<VertexAttribute> VertexAttributeRef;

/** Base class for the single vertex attribute that provides
 the name and type of a vertex attribute.
 */
class SingleVertexAttributeInfo
{
public:
    SingleVertexAttributeInfo();
    SingleVertexAttributeInfo(StringIdentity nameID,int slot,BDAttributeDataType type);
    
    /// Comparison operator for set
    bool operator < (const SingleVertexAttributeInfo &that) const
    {
        if (nameID == that.nameID)
            return type < that.type;
        return nameID < that.nameID;
    }
    
    bool operator == (const SingleVertexAttributeInfo &that) const
    {
        bool ret = (nameID == that.nameID);
        if (ret)
            return type == that.type;
        return ret;
    }
    
    /// Return the size for the particular data type
    int size() const;
    
    /// Attribute's data type
    BDAttributeDataType type;
    
    /// Buffer slot for Metal
    int slot;
    
    /// Attribute name (e.g. "u_elev")
    StringIdentity nameID;
};

typedef std::set<SingleVertexAttributeInfo> SingleVertexAttributeInfoSet;

/** The single vertex attribute holds a single typed value to
 be merged into a basic drawable's attributes arrays.
 */
class SingleVertexAttribute : public SingleVertexAttributeInfo
{
public:
    SingleVertexAttribute();
    SingleVertexAttribute(StringIdentity nameID,int slot,float floatVal);
    SingleVertexAttribute(StringIdentity nameID,int slot,int intVal);
    SingleVertexAttribute(StringIdentity nameID,int slot,int64_t intVal);
    SingleVertexAttribute(StringIdentity nameID,int slot,unsigned char colorVal[4]);
    SingleVertexAttribute(StringIdentity nameID,int slot,float vec0,float vec1);
    SingleVertexAttribute(StringIdentity nameID,int slot,float vec0,float vec1,float vec2);
    SingleVertexAttribute(StringIdentity nameID,int slot,float vec0,float vec1,float vec2, float vec3);
    
    /// The actual data
    union {
        float vec4[4];
        float vec3[3];
        float vec2[2];
        float floatVal;
        unsigned char color[4];
        int intVal;
        int64_t int64Val;
    } data;
};

typedef std::set<SingleVertexAttribute> SingleVertexAttributeSet;

/// Generate a vertex attribute info set from a vertex attribute set (e.g. strip out the values)
void VertexAttributeSetConvert(const SingleVertexAttributeSet &attrSet,SingleVertexAttributeInfoSet &infoSet);

/// See if the given set of attributes is compatible with the set of attributes (without data)
bool VertexAttributesAreCompatible(const SingleVertexAttributeInfoSet &infoSet,const SingleVertexAttributeSet &attrSet);
    
}
