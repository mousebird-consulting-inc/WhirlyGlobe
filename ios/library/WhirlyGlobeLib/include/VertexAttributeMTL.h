/*
 *  VertexAttributeMTL.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/16/19.
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

#import "WrapperMTL.h"
#import "VertexAttribute.h"

namespace WhirlyKit
{
class BufferEntryMTL;
    
// Metal wrapper around vertex attribute
class VertexAttributeMTL : public VertexAttribute
{
public:
    VertexAttributeMTL(BDAttributeDataType dataType,StringIdentity nameID);
    VertexAttributeMTL(const VertexAttributeMTL &that);
    
    // In memory size of the data component
    int sizeMTL() const;
    
    // Low level data type
    MTLVertexFormat formatMTL() const;
public:
    // If set, the Metal buffer we've set up for this one
    BufferEntryMTL buffer;
};

class SingleVertexAttributeInfoMTL : public SingleVertexAttributeInfo
{
public:
    // Construct with the basic info
    SingleVertexAttributeInfoMTL(const SingleVertexAttributeInfo &that);
    
//    /// Return the number of components as needed by glVertexAttribPointer
//    GLuint glEntryComponents() const;
//
//    /// Return the data type as required by glVertexAttribPointer
//    GLenum glType() const;
//
//    /// Whether or not glVertexAttribPointer will normalize the data
//    GLboolean glNormalize() const;
};
    
}
