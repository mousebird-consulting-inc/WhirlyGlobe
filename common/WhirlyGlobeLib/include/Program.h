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
    
    /// Used only for comparison
    Program(SimpleIdentity theId);
    
    /// Return true if it was built correctly
    virtual bool isValid() = 0;

    /// Set the given uniform to the given value.
    /// These check the type and cache a value to save on duplicate gl calls
    virtual bool setUniform(StringIdentity nameID,float val) = 0;
    virtual bool setUniform(StringIdentity nameID,float val,int index) = 0;
    virtual bool setUniform(StringIdentity nameID,const Eigen::Vector2f &vec) = 0;
    virtual bool setUniform(StringIdentity nameID,const Eigen::Vector3f &vec) = 0;
    virtual bool setUniform(StringIdentity nameID,const Eigen::Vector4f &vec) = 0;
    virtual bool setUniform(StringIdentity nameID,const Eigen::Vector4f &vec,int index) = 0;
    virtual bool setUniform(StringIdentity nameID,const Eigen::Matrix4f &mat) = 0;
    virtual bool setUniform(StringIdentity nameID,int val) = 0;
    virtual bool setUniform(const SingleVertexAttribute &attr) = 0;
    
    /// Check for the specific attribute associated with WhirlyKit lights
    virtual bool hasLights() = 0;
    
    /// Tie a given texture ID to the given name.
    /// We have to set these up each time before drawing
    virtual bool setTexture(StringIdentity nameID,TextureBase *tex) = 0;

    /// Return the name (for tracking purposes)
    const std::string &getName();
    
    /// Clean up renderer resources
    virtual void teardownForRenderer(const RenderSetupInfo *setupInfo) = 0;
    
protected:
    std::string name;
    TimeInterval lightsLastUpdated;
};
    
typedef std::shared_ptr<Program> ProgramRef;

/// Set a texture ID by name in a Shader (Program)
class ShaderAddTextureReq : public ChangeRequest
{
public:
    ShaderAddTextureReq(SimpleIdentity shaderID,SimpleIdentity nameID,SimpleIdentity texID);
    
    void execute(Scene *scene,SceneRenderer *renderer,WhirlyKit::View *view);

protected:
    SimpleIdentity shaderID;
    SimpleIdentity nameID;
    SimpleIdentity texID;
};

}
