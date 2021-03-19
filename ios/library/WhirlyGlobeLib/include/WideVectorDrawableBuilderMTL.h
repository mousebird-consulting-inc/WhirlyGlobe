/*
 *  WideVectorDrawableBuilderMTL.h
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

#import "WideVectorDrawableBuilder.h"
#import "BaseInfo.h"

namespace WhirlyKit
{
    
/// Metal version of the WideVectorDrawable Builder
class WideVectorDrawableBuilderMTL : virtual public WideVectorDrawableBuilder
{
public:
    WideVectorDrawableBuilderMTL(const std::string &name,Scene *scene);
    
    // Initialize with an estimate on the number of vertices and triangles
    virtual void Init(unsigned int numVert,unsigned int numTri,bool globeMode) override;
        
    WideVectorTweaker *makeTweaker() override;

    virtual DrawableRef getDrawable();
    
protected:
};

}
