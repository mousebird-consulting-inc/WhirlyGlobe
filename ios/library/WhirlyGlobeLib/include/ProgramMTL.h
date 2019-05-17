/*
 *  ProgramMTL.h
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
#import "Program.h"

namespace WhirlyKit
{

/** Representation of an iOS Metal.  It's an identifiable so we can
 point to it generically.  Otherwise, pretty basic.
 */
class ProgramMTL : public Program
{
public:
    ProgramMTL();
    virtual ~ProgramMTL();
    
    /// Used only for comparison
    ProgramMTL(SimpleIdentity theId) : Program(theId), lightsLastUpdated(0.0) { }
    
    /// Initialize with both shader programs
    /// We'll look for both mains
    ProgramMTL(const std::string &name,const std::string &progStr);
    
    /// Return true if it was built correctly
    bool isValid();
    
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
    bool setTexture(StringIdentity nameID,TextureBase *tex);
    
    /// Check for the specific attribute associated with WhirlyKit lights
    bool hasLights();
    
    /// Set the attributes associated with lighting.
    /// We'll check their last updated time against ours.
    bool setLights(const std::vector<DirectionalLight> &lights, TimeInterval lastUpdated, Material *mat, Eigen::Matrix4f &modelMat);
        
    /// Return the name (for tracking purposes)
    const std::string &getName() { return name; }
    
    /// Return the GL Program ID
    id<MTLLibrary> getProgram() { return program; }
    
    /// Bind any program specific textures right before we draw.
    /// We get to start at 0 and return however many we bound
    int bindTextures();
    
    /// Clean up OpenGL resources, rather than letting the destructor do it (which it will)
    virtual void teardownForRenderer(const RenderSetupInfo *setupInfo);
    void cleanUp();
    
protected:
    id<MTLLibrary> program;
    std::string name;
    TimeInterval lightsLastUpdated;
};

    
    
}
