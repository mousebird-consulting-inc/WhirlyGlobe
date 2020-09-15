/*
 *  ParticleSystemDrawableBuilderGLES.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/14/19.
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

#import "ParticleSystemDrawableBuilderGLES.h"

namespace WhirlyKit
{

ParticleSystemDrawableBuilderGLES::ParticleSystemDrawableBuilderGLES(const std::string &name,Scene *scene)
    : ParticleSystemDrawableBuilder(name,scene), drawableGotten(false)
{
    draw = new ParticleSystemDrawableGLES(name);
}
    
void ParticleSystemDrawableBuilderGLES::setup(const std::vector<SingleVertexAttributeInfo> &inVertAttrs,
                   const std::vector<SingleVertexAttributeInfo> &inVaryAttrs,
                   const std::vector<SimpleIdentity> &inVaryNames,
                   int numTotalPoints,int batchSize,int vertexSize,bool useRectangles,bool useInstancing)
{
    ParticleSystemDrawableGLES *drawGL = dynamic_cast<ParticleSystemDrawableGLES *>(draw);

    for (auto attr : inVertAttrs)
    {
        drawGL->vertexSize += attr.size();
        drawGL->vertAttrs.push_back(SingleVertexAttributeInfoGLES(attr));
    }
    for (auto varyAttr : inVaryAttrs)
    {
        drawGL->varyAttrs.push_back(SingleVertexAttributeInfoGLES(varyAttr));
    }
    drawGL->varyNames = inVaryNames;

    ParticleSystemDrawableBuilder::setup(inVertAttrs,inVaryAttrs,inVaryNames,numTotalPoints,batchSize,drawGL->vertexSize,useRectangles,useInstancing);
}
    
ParticleSystemDrawableBuilderGLES::~ParticleSystemDrawableBuilderGLES()
{
    if (!drawableGotten && draw)
        delete draw;
}
    
ParticleSystemDrawable *ParticleSystemDrawableBuilderGLES::getDrawable()
{
    if (!draw)
        return NULL;
    
    if (!drawableGotten) {
        drawableGotten = true;
    }
    
    return draw;
}
    
}
