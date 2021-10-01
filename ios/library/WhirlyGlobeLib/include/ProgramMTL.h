/*  ProgramMTL.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/16/19.
 *  Copyright 2011-2021 mousebird consulting
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
 */

#import <Foundation/Foundation.h>
#import "WrapperMTL.h"
#import "Program.h"

namespace WhirlyKit
{
    
class RendererFrameInfoMTL;
class SceneMTL;

/** Representation of an iOS Metal.  It's an identifiable so we can
 point to it generically.  Otherwise, pretty basic.
 */
class ProgramMTL : public Program
{
public:
    ProgramMTL() = default;
    virtual ~ProgramMTL() = default;
    
    /// Set up with a vertex and fragment shader
    ProgramMTL(const std::string &name,id<MTLFunction> vertfunc,id<MTLFunction> fragFunc);
    
    /// Return true if it was built correctly
    virtual bool isValid() const override;
    
    /// Check for the specific attribute associated with WhirlyKit lights
    virtual bool hasLights() const override;
    
    /// Set the attributes associated with lighting.
    /// We'll check their last updated time against ours.
    virtual bool setLights(const std::vector<DirectionalLight> &lights, TimeInterval lastUpdated,
                           const Material *mat, const Eigen::Matrix4f &modelMat) const;
    
    /// Tie a given texture ID to the given slot in the renderer
    /// We have to set these up each time before drawing
    virtual bool setTexture(StringIdentity nameID,TextureBase *tex,int textureSlot) override;
    
    /// Remove the corresponding texture entries
    virtual void clearTexture(SimpleIdentity texID) override;
    
    /// Return the name (for tracking purposes)
    virtual const std::string &getName() const override;
    
    /// Clean up Metal resources, rather than letting the destructor do it (which it will)
    virtual void teardownForRenderer(const RenderSetupInfo *setupInfo,Scene *scene,RenderTeardownInfoRef teardown) override;

public:
    bool valid = false;
    id<MTLFunction> vertFunc,fragFunc;
    TimeInterval lightsLastUpdated = 0.0;

    // Program wide textures
    class TextureEntry {
    public:
        TextureEntry() = default;
        
        int slot = -1;
        TextureEntryMTL texBuf;
        SimpleIdentity texID;
    } ;
    std::vector<TextureEntry> textures;
};
    
typedef std::shared_ptr<ProgramMTL> ProgramMTLRef;
    
}
