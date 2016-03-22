/*
 *  OpenGLES2Program.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/23/12.
 *  Copyright 2011-2016 mousebird consulting
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
#import "glwrapper.h"
#import "WhirlyTypes.h"
#import "Identifiable.h"
#import "WhirlyVector.h"


namespace WhirlyKit
{

class WhirlyKitDirectionalLight;
class WhirlyKitMaterial;

/// Used to track a uniform within an OpenGL ES 2.0 shader program
class OpenGLESUniform
{
public:
    OpenGLESUniform() : index(0), size(0), isSet(false), isTexture(false) { }
    OpenGLESUniform(const std::string &name) : name(name) { }
    
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
    /// Set if we know this is a texture
    bool isTexture;
        
    /// Current value (if set)
    bool isSet;
    union {
        int iVals[4];
        float fVals[4];
        float mat[16];
    } val;
};

// Used for sorting
typedef struct
{
    bool operator()(const OpenGLESUniform *a,const OpenGLESUniform *b) const
    {
        return a->name < b->name;
    }
} UniformNameSortStruct;

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
        
// Used for sorting
typedef struct
{
    bool operator()(const OpenGLESAttribute *a,const OpenGLESAttribute *b) const
    {
        return a->name < b->name;
    }
} AttributeNameSortStruct;


/** Representation of an OpenGL ES 2.0 program.  It's an identifiable so we can
    point to it generically.  Otherwise, pretty basic.
 */
class OpenGLES2Program : public Identifiable
{
public:
    OpenGLES2Program();
    virtual ~OpenGLES2Program();
    
    /// Used only for comparison
    OpenGLES2Program(SimpleIdentity theId) : Identifiable(theId), lightsLastUpdated(0.0) { }

    /// Initialize with both shader programs
    OpenGLES2Program(const std::string &name,const std::string &vShaderString,const std::string &fShaderString);
    
    /// Return true if it was built correctly
    bool isValid();
        
    /// Search for the given uniform name and return the info.  NULL on failure.
    OpenGLESUniform *findUniform(const std::string &uniformName);

    /// Set the given uniform to the given value.
    /// These check the type and cache a value to save on duplicate gl calls
    bool setUniform(const std::string &name,float val);
    bool setUniform(const std::string &name,const Eigen::Vector2f &vec);
    bool setUniform(const std::string &name,const Eigen::Vector3f &vec);
    bool setUniform(const std::string &name,const Eigen::Vector4f &vec);
    bool setUniform(const std::string &name,const Eigen::Matrix4f &mat);
    bool setUniform(const std::string &name,int val);
    
    /// Tie a given texture ID to the given name.
    /// We have to set these up each time before drawing
    bool setTexture(const std::string &name,GLuint val);
    
    /// Check for the specific attribute associated with WhirlyKit lights
    bool hasLights();
    
    /// Set the attributes associated with lighting.
    /// We'll check their last updated time against ours.
    bool setLights(std::vector<WhirlyKitDirectionalLight*> lights, TimeInterval lastUpdated, WhirlyKitMaterial *mat, Eigen::Matrix4f &modelMat);
    
    /// Search for the given attribute name and return the info.  NULL on failure.
    const OpenGLESAttribute *findAttribute(const std::string &attrName);
    
    /// Return the name (for tracking purposes)
    const std::string &getName() { return name; }
    
    /// Return the GL Program ID
    GLuint getProgram() { return program; }
    
    /// Bind any program specific textures right before we draw.
    /// We get to start at 0 and return however many we bound
    int bindTextures();

    /// Clean up OpenGL resources, rather than letting the destructor do it (which it will)
    void cleanUp();

protected:
    std::string name;
    GLuint program;
    GLuint vertShader;
    GLuint fragShader;
    TimeInterval lightsLastUpdated;
    // Uniforms sorted for fast lookup
    std::set<OpenGLESUniform *,UniformNameSortStruct> uniforms;
    // Attributes sorted for fast lookup
    std::set<OpenGLESAttribute *,AttributeNameSortStruct> attrs;
};
    
}
