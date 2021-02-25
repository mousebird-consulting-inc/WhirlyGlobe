/*
 *  ParticleSystemDrawableBuilderMTL.mm
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

#import "ParticleSystemDrawableBuilderMTL.h"

namespace WhirlyKit
{

ParticleSystemDrawableBuilderMTL::ParticleSystemDrawableBuilderMTL(const std::string &name,Scene *scene)
: ParticleSystemDrawableBuilder(name,scene), drawableGotten(false)
{
    draw = new ParticleSystemDrawableMTL(name);
}

void ParticleSystemDrawableBuilderMTL::setup(const std::vector<SingleVertexAttributeInfo> &inVertAttrs,
                                              const std::vector<SingleVertexAttributeInfo> &inVaryAttrs,
                                             const std::vector<SimpleIdentity> &inVaryNames,
                                              int numTotalPoints,int batchSize,int vertexSize,bool useRectangles,bool useInstancing)
{
    ParticleSystemDrawableBuilder::setup(inVertAttrs,inVaryAttrs,inVaryNames,numTotalPoints,batchSize,vertexSize,useRectangles,useInstancing);
}

ParticleSystemDrawableBuilderMTL::~ParticleSystemDrawableBuilderMTL()
{
    if (!drawableGotten && draw)
        delete draw;
}

ParticleSystemDrawable *ParticleSystemDrawableBuilderMTL::getDrawable()
{
    if (!draw)
        return NULL;
    
    drawableGotten = true;

    return draw;
}
    
}
