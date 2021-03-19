/*  BillboardDrawableBuilderMTL.h
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

#import "BillboardDrawableBuilder.h"
#import "BasicDrawableBuilderMTL.h"

namespace WhirlyKit
{

// Passes in the uniform values the shader is expecting
struct BillboardTweakerMTL : public BasicDrawableTweaker
{
    BillboardTweakerMTL();
    
    virtual void tweakForFrame(Drawable *inDraw,RendererFrameInfo *frameInfo) override;
    
    bool groundMode;
};
    
/** Metal version of BillboardDrawable Builder
 */
class BillboardDrawableBuilderMTL : virtual public BasicDrawableBuilderMTL, virtual public BillboardDrawableBuilder
{
public:
    BillboardDrawableBuilderMTL(const std::string &name,Scene *scene);
    
    virtual void Init() override;
    
    virtual BasicDrawableRef getDrawable() override;
};

}
