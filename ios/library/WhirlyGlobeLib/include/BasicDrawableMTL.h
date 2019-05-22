/*
 *  BasicDrawableMTL.h
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

#import "Identifiable.h"
#import "WhirlyVector.h"
#import "BasicDrawable.h"
#import "WrapperMTL.h"
#import "VertexAttributeMTL.h"

namespace WhirlyKit
{
    
/** Metal Version of the BasicDrawable.
 */
class BasicDrawableMTL : public BasicDrawable
{
public:
    BasicDrawableMTL(const std::string &name);

    // Note: Overriding for debugging
    virtual bool isOn(RendererFrameInfo *frameInfo) const;
    
    /// Set up local rendering structures (e.g. VBOs)
    virtual void setupForRenderer(const RenderSetupInfo *setupInfo);
    
    /// Clean up any rendering objects you may have (e.g. VBOs).
    virtual void teardownForRenderer(const RenderSetupInfo *setupInfo);
    
    /// Fill this in to draw the basic drawable
    /// Note: Make this GL only
    virtual void draw(RendererFrameInfo *frameInfo,Scene *scene);
    
public:
    bool setupForMTL;
    std::vector<Triangle> tris;
    int numPts,numTris;
    id<MTLBuffer> triBuffer;
};

    
}
