/*
 *  OpenGLES2Program.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/23/12.
 *  Copyright 2011-2012 mousebird consulting
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

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import "Identifiable.h"
#import <vector>

namespace WhirlyKit
{
    
/// Used to track a uniform within an OpenGL ES 2.0 shader program
class OpenGLESUniform
{
public:
    OpenGLESUniform() : index(0), size(0) { }
    OpenGLESUniform(const std::string &name) : name(name) { }
    bool operator < (const OpenGLESUniform &that) const { return name < that.name; }
    
    /// Return true if this uniform is an array
    bool isArray() { return size != 0; }
    /// Return true if the type matches
    bool isType(GLenum inType) { return inType == type; }
    
    /// Name of the uniform within the program
    std::string name;
    /// Index we'll use to address the uniform
    GLuint index;
    /// If the uniform is an array, this is the length
    GLint size;
    /// Uniform data type
    GLenum type;
};
        
/// Used to track an attribute (per vertex) within an OpenGL ES 2.0 shader program
class OpenGLESAttribute
{
public:
    OpenGLESAttribute() : index(0), size(0) { }
    OpenGLESAttribute(const std::string &name) : name(name) { }
    bool operator < (const OpenGLESAttribute &that) const { return name < that.name; }
    
    /// Return true if this uniform is an array
    bool isArray() { return size != 0; }
    /// Return true if the type matches
    bool isType(GLenum inType) { return inType == type; }

    /// Name of the per vertex attribute
    std::string name;
    /// Index we'll use to address the attribute
    GLuint index;
    /// If an array, this is the length
    GLint size;
    /// Attribute data type
    GLenum type;
};

/** Representation of an OpenGL ES 2.0 program.  It's an identifiable so we can
    point to it generically.  Otherwise, pretty basic.
 */
class OpenGLES2Program : public Identifiable
{
public:
    /// Used only for comparison
    OpenGLES2Program(SimpleIdentity theId) : Identifiable(theId) { }

    /// Initialize with both shader programs
    OpenGLES2Program(const std::string &name,const std::string &vShaderString,const std::string &fShaderString);
    
    /// Return true if it was built correctly
    bool isValid();
        
    /// Search for the given uniform name and return the info.  NULL on failure.
    const OpenGLESUniform *findUniform(const std::string &uniformName);
    
    /// Search for the given attribute name and return the info.  NULL on failure.
    const OpenGLESAttribute *findAttribute(const std::string &attrName);
    
    /// Return the name (for tracking purposes)
    const std::string &getName() { return name; }
    
    /// Return the GL Program ID
    GLuint getProgram() { return program; }

    /// Clean up OpenGL resources, rather than letting the destructor do it (which it will)
    void cleanUp();

protected:
    std::string name;
    GLuint program;
    GLuint vertShader;
    GLuint fragShader;
    // Uniforms sorted for fast lookup
    std::set<OpenGLESUniform> uniforms;
    // Attributes sorted for fast lookup
    std::set<OpenGLESAttribute> attrs;
};

}
