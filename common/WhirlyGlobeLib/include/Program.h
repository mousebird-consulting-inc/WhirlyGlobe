/*
 *  Program.h
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
#import "Identifiable.h"
#import "WhirlyVector.h"
#import "Drawable.h"
#import "VertexAttribute.h"
#import "Texture.h"

namespace WhirlyKit
{
    
class DirectionalLight;
class Material;

/** Representation of an OpenGL ES 2.0 program.  It's an identifiable so we can
    point to it generically.  Otherwise, pretty basic.
 */
class Program : public Identifiable
{
public:
    Program();
    virtual ~Program();
        
    /// Return true if it was built correctly
    virtual bool isValid() = 0;
    
    /// Check for the specific attribute associated with WhirlyKit lights
    virtual bool hasLights() = 0;
    
    /// Return the name (for tracking purposes)
    const std::string &getName();
    
    /// Tie a given texture ID to the given slot or nameID (depending on renderer)
    /// We have to set these up each time before drawing
    virtual bool setTexture(StringIdentity nameID,TextureBase *tex,int textureSlot) = 0;
    
    /// Set a block of uniforms (Metal only, at the moment)
    virtual void setUniBlock(const BasicDrawable::UniformBlock &uniBlock);

    /// Clean up renderer resources
    virtual void teardownForRenderer(const RenderSetupInfo *setupInfo,Scene *scene) = 0;
    
    // Reduce operation modes
    typedef enum {None,TextureReduce} ReduceMode;
    
    // If not set to None, this shader is expecting to be run over a mipmap'ed, one level at a time
    virtual void setReduceMode(ReduceMode reduceMode);

    // Current reduce mode (or off)
    virtual ReduceMode getReduceMode();
    
public:
    std::string name;
    TimeInterval lightsLastUpdated;
    ReduceMode reduceMode;
    // Uniforms to be passed into a shader (just Metal for now)
    std::vector<BasicDrawable::UniformBlock> uniBlocks;
};
    
typedef std::shared_ptr<Program> ProgramRef;

/// Set a texture ID by name in a Shader (Program)
class ShaderAddTextureReq : public ChangeRequest
{
public:
    /// Add the given texture to the given shader.  For Metal, we need a texture slot.  Ignore for OpenGL
    ShaderAddTextureReq(SimpleIdentity shaderID,SimpleIdentity nameID,SimpleIdentity texID,int textureSlot);
    
    void execute(Scene *scene,SceneRenderer *renderer,WhirlyKit::View *view);

protected:
    SimpleIdentity shaderID;
    SimpleIdentity nameID;
    SimpleIdentity texID;
    int textureSlot;
};

/// Add a uniform block to a whole Program (rather than geometry)
class ProgramUniformBlockSetRequest : public ChangeRequest
{
public:
    ProgramUniformBlockSetRequest(SimpleIdentity progID,const RawDataRef &uniBlock,int bufferID);
    ~ProgramUniformBlockSetRequest() { }

    /// Remove from the renderer.  Never call this.
    void execute(Scene *scene,SceneRenderer *renderer,View *view);

protected:
    SimpleIdentity progID;
    BasicDrawable::UniformBlock uniBlock;
};

}
