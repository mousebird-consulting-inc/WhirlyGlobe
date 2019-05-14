/*
 *  VertexAttributeGLES.h
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
#import "WrapperGLES.h"

namespace WhirlyKit
{
    
/// Used to keep track of attributes (other than points)
class VertexAttributeGLES : public VertexAttribute
{
public:
    VertexAttributeGLES(BDAttributeDataType dataType,StringIdentity nameID);
    VertexAttributeGLES(const VertexAttributeGLES &that);

    /// Return the number of components as needed by glVertexAttribPointer
    GLuint glEntryComponents() const;
    
    /// Return the data type as required by glVertexAttribPointer
    GLenum glType() const;
    
    /// Whether or not glVertexAttribPointer will normalize the data
    GLboolean glNormalize() const;
    
    /// Calls glVertexAttrib* for the appropriate type
    void glSetDefault(int index) const;
    
public:
    /// Buffer offset within interleaved vertex
    GLuint buffer;
};
    
/** Base class for the single vertex attribute that provides
 the name and type of a vertex attribute.
 */
class SingleVertexAttributeInfoGLES : public SingleVertexAttributeInfo
{
public:
    // Construct with the basic info
    SingleVertexAttributeInfoGLES(const SingleVertexAttributeInfo &that);
    
    /// Return the number of components as needed by glVertexAttribPointer
    GLuint glEntryComponents() const;
    
    /// Return the data type as required by glVertexAttribPointer
    GLenum glType() const;
    
    /// Whether or not glVertexAttribPointer will normalize the data
    GLboolean glNormalize() const;
};

}
