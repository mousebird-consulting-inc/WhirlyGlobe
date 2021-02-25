/*
 *  OpenGLES2Program.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/23/12.
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
#import <unordered_map>
#import "UtilsGLES.h"
#import "Identifiable.h"
#import "WhirlyVector.h"
#import "Drawable.h"
#import "Program.h"

namespace WhirlyKit
{
    
class DirectionalLight;
class Material;

/// Used to track a uniform within an OpenGL ES 2.0 shader program
class OpenGLESUniform
{
public:
    OpenGLESUniform() : index(0), size(0), isSet(false), isTexture(false) { }
    OpenGLESUniform(StringIdentity nameID) : nameID(nameID) { }
    
    bool operator < (const OpenGLESUniform &that) const { return nameID < that.nameID; }
    
    /// Return true if this uniform is an array
    bool isArray() { return size != 0; }
    /// Return true if the type matches
    bool isType(GLenum inType) { return inType == type; }
    
    /// Name of the uniform within the program
    StringIdentity nameID;
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

/// Used to track an attribute (per vertex) within an OpenGL ES 2.0 shader program
class OpenGLESAttribute
{
public:
    OpenGLESAttribute() : index(0), size(0) { }
    OpenGLESAttribute(StringIdentity nameID) : nameID(nameID) { }
    
    bool operator < (const OpenGLESAttribute &that) const { return nameID < that.nameID; }
    
    /// Return true if this uniform is an array
    bool isArray() { return size != 0; }
    /// Return true if the type matches
    bool isType(GLenum inType) { return inType == type; }
    
    /// Name of the per vertex attribute
    StringIdentity nameID;
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
class ProgramGLES : public Program
{
public:
    ProgramGLES();
    virtual ~ProgramGLES();
        
    /// Initialize with both shader programs
    ProgramGLES(const std::string &name,const std::string &vShaderString,const std::string &fShaderString,const std::vector<std::string> *varyings=NULL);
    
    /// Return true if it was built correctly
    bool isValid() override;
    
    /// Search for the given uniform name and return the info.  NULL on failure.
    OpenGLESUniform *findUniform(StringIdentity nameID);
    
    /// Set the given uniform to the given value.
    /// These check the type and cache a value to save on duplicate gl calls
    bool setUniform(StringIdentity nameID,float val);
    bool setUniform(StringIdentity nameID,float val,int index);
    bool setUniform(StringIdentity nameID,const Eigen::Vector2f &vec);
    bool setUniform(StringIdentity nameID,const Eigen::Vector3f &vec);
    bool setUniform(StringIdentity nameID,const Eigen::Vector4f &vec);
    bool setUniform(StringIdentity nameID,const Eigen::Vector4f &vec,int index);
    bool setUniform(StringIdentity nameID,const Eigen::Matrix4f &mat);
    bool setUniform(StringIdentity nameID,int val);
    bool setUniform(const SingleVertexAttribute &attr);
    
    /// Tie a given texture ID to the given name.
    /// We have to set these up each time before drawing
    /// textureSlot is ignored OpenGLES
    bool setTexture(StringIdentity nameID,TextureBase *tex,int textureSlot) override;
    
    /// Remove a texture entry
    void clearTexture(SimpleIdentity texID) override;
    
    /// Check for the specific attribute associated with WhirlyKit lights
    bool hasLights() override;
    
    /// Set the attributes associated with lighting.
    /// We'll check their last updated time against ours.
    bool setLights(const std::vector<DirectionalLight> &lights, TimeInterval lastUpdated, Material *mat, Eigen::Matrix4f &modelMat);
    
    /// Search for the given attribute name and return the info.  NULL on failure.
    const OpenGLESAttribute *findAttribute(StringIdentity nameID);
    
    /// Return the GL Program ID
    GLuint getProgram() { return program; }
    
    /// Bind any program specific textures right before we draw.
    /// We get to start at 0 and return however many we bound
    int bindTextures();
    
    /// Clean up OpenGL resources, rather than letting the destructor do it (which it will)
    virtual void teardownForRenderer(const RenderSetupInfo *setupInfo,Scene *scene,RenderTeardownInfoRef teardown) override;
    void cleanUp();
    
protected:
    GLuint program;
    GLuint vertShader;
    GLuint fragShader;
    TimeInterval lightsLastUpdated;
    // Uniforms sorted for fast lookup
    std::unordered_map<StringIdentity,std::shared_ptr<OpenGLESUniform>> uniforms;
    // Attributes sorted for fast lookup
    std::unordered_map<StringIdentity,std::shared_ptr<OpenGLESAttribute>> attrs;
};
    
typedef std::shared_ptr<ProgramGLES> ProgramGLESRef;
    
}
