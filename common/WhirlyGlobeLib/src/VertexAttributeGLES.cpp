/*
 *  VertexAttributeGLES.cpp
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

#import "VertexAttributeGLES.h"

using namespace Eigen;

namespace WhirlyKit
{
    
VertexAttributeGLES::VertexAttributeGLES(BDAttributeDataType dataType,StringIdentity nameID)
    : VertexAttribute(dataType,-1,nameID), buffer(0)
    {
    }
    
VertexAttributeGLES::VertexAttributeGLES(const VertexAttributeGLES &that)
    : VertexAttribute(that), buffer(that.buffer)
{
}
    
/// Return the number of components as needed by glVertexAttribPointer
GLuint VertexAttributeGLES::glEntryComponents() const
{
    switch (dataType)
    {
        case BDFloat4Type:
            return 4;
            break;
        case BDFloat3Type:
            return 3;
            break;
        case BDFloat2Type:
            return 2;
            break;
        case BDChar4Type:
            return 4;
            break;
        case BDFloatType:
            return 1;
            break;
        case BDIntType:
            return 1;
            break;
        case BDDataTypeMax:
            return 0;
            break;
    }
    
    return 0;
}
    
SingleVertexAttributeInfoGLES::SingleVertexAttributeInfoGLES(const SingleVertexAttributeInfo &that)
    : SingleVertexAttributeInfo(that)
{
}

/// Return the number of components as needed by glVertexAttribPointer
GLuint SingleVertexAttributeInfoGLES::glEntryComponents() const
{
    switch (type)
    {
        case BDFloat4Type:
            return 4;
            break;
        case BDFloat3Type:
            return 3;
            break;
        case BDFloat2Type:
            return 2;
            break;
        case BDChar4Type:
            return 4;
            break;
        case BDFloatType:
            return 1;
            break;
        case BDIntType:
            return 1;
            break;
        case BDDataTypeMax:
            break;
    }
    
    return 0;
}

/// Return the data type as required by glVertexAttribPointer
GLenum VertexAttributeGLES::glType() const
{
    switch (dataType)
    {
        case BDFloat4Type:
        case BDFloat3Type:
        case BDFloat2Type:
        case BDFloatType:
            return GL_FLOAT;
            break;
        case BDChar4Type:
            return GL_UNSIGNED_BYTE;
            break;
        case BDIntType:
            return GL_INT;
            break;
        case BDDataTypeMax:
            return GL_INT;
            break;
    }
    return GL_UNSIGNED_BYTE;
}

/// Return the data type as required by glVertexAttribPointer
GLenum SingleVertexAttributeInfoGLES::glType() const
{
    switch (type)
    {
        case BDFloat4Type:
        case BDFloat3Type:
        case BDFloat2Type:
        case BDFloatType:
            return GL_FLOAT;
            break;
        case BDChar4Type:
            return GL_UNSIGNED_BYTE;
            break;
        case BDIntType:
            return GL_INT;
            break;
        case BDDataTypeMax:
            return GL_INT;
            break;
    }
    return GL_UNSIGNED_BYTE;
}

/// Whether or not glVertexAttribPointer will normalize the data
GLboolean VertexAttributeGLES::glNormalize() const
{
    switch (dataType)
    {
        case BDFloat4Type:
        case BDFloat3Type:
        case BDFloat2Type:
        case BDFloatType:
        case BDIntType:
            return GL_FALSE;
            break;
        case BDChar4Type:
            return GL_TRUE;
            break;
        case BDDataTypeMax:
            return GL_FALSE;
            break;
    }
}

/// Whether or not glVertexAttribPointer will normalize the data
GLboolean SingleVertexAttributeInfoGLES::glNormalize() const
{
    switch (type)
    {
        case BDFloat4Type:
        case BDFloat3Type:
        case BDFloat2Type:
        case BDFloatType:
        case BDIntType:
            return GL_FALSE;
            break;
        case BDChar4Type:
            return GL_TRUE;
            break;
        case BDDataTypeMax:
            return GL_FALSE;
            break;
    }
}

void VertexAttributeGLES::glSetDefault(int index) const
{
    switch (dataType)
    {
        case BDFloat4Type:
            glVertexAttrib4f(index, defaultData.vec4[0], defaultData.vec4[1], defaultData.vec4[2], defaultData.vec4[3]);
            break;
        case BDFloat3Type:
            glVertexAttrib3f(index, defaultData.vec3[0], defaultData.vec3[1], defaultData.vec3[2]);
            break;
        case BDFloat2Type:
            glVertexAttrib2f(index, defaultData.vec2[0], defaultData.vec2[1]);
            break;
        case BDFloatType:
            glVertexAttrib1f(index, defaultData.floatVal);
            break;
        case BDChar4Type:
            glVertexAttrib4f(index, defaultData.color[0] / 255.0, defaultData.color[1] / 255.0, defaultData.color[2] / 255.0, defaultData.color[3] / 255.0);
            break;
        case BDIntType:
            glVertexAttrib1f(index, defaultData.intVal);
            break;
        case BDDataTypeMax:
            break;
    }
}
    
}
