/*  ParticleSystemDrawable.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/28/15.
 *  Copyright 2011-2022 mousebird consulting
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

#import "ParticleSystemDrawable.h"
#import "SceneRenderer.h"
#import "WhirlyKitLog.h"
#import "BaseInfo.h"

namespace WhirlyKit
{

ParticleSystemDrawable::ParticleSystemDrawable(const std::string &name)
    : Drawable(name)
{
}

void ParticleSystemDrawable::setUniBlock(const BasicDrawable::UniformBlock &uniBlock)
{
    for (auto & ii : uniBlocks)
        if (ii.bufferID == uniBlock.bufferID) {
            ii = uniBlock;
            return;
        }
    
    uniBlocks.push_back(uniBlock);
}
        
void ParticleSystemDrawable::updateBatches(TimeInterval now)
{
    {
        std::lock_guard<std::mutex> guardLock(batchLock);
        // Check the batches to see if any have gone off
        for (int bi=startb;bi<endb;)
        {
            Batch &batch = batches[bi % batches.size()];
            if (batch.active)
            {
                if (batch.startTime + lifetime < now)
                {
                    batch.active = false;
                    chunksDirty = true;
                    startb++;
                }
            } else
                break;
            
            bi++;
        }
    }
    
    updateChunks();
}
    
void ParticleSystemDrawable::updateChunks()
{
    if (!chunksDirty)
        return;
    
    std::lock_guard<std::mutex> guardLock(batchLock);

    chunksDirty = false;
    chunks.clear();
    if (startb != endb)
    {
        int start = 0;
        do {
            // Skip empty batches at the beginning
            for (;start < batches.size() && !batches[start].active;start++);

            int end = start;
            if (start < batches.size())
            {
                for (;end < batches.size() && batches[end].active;end++);
                if (start != end)
                {
                    BufferChunk chunk;
                    chunk.bufferStart = (start % (int)batches.size()) * batchSize * vertexSize;
                    chunk.vertexStart = (start % (int)batches.size()) * batchSize;
                    chunk.numVertices = (end-start) * batchSize;
                    chunks.push_back(chunk);
                }
            }
            
            start = end;
        } while (start < batches.size());
    }
}
    
void ParticleSystemDrawable::setupBatches()
{
    // Set up the batches
    int numBatches = numTotalPoints / batchSize;
    int batchBufLen = batchSize * vertexSize;
    batches.resize(numBatches);
    unsigned int bufOffset = 0;
    for (unsigned int ii=0;ii<numBatches;ii++)
    {
        Batch &batch = batches[ii];
        batch.active = false;
        batch.batchID = ii;
        batch.offset = bufOffset;
        batch.len = batchBufLen;
        bufOffset += batchBufLen;
    }
    chunks.clear();
    chunksDirty = true;
}
    
bool ParticleSystemDrawable::findEmptyBatch(Batch &retBatch)
{
    bool ret = false;
    
    std::lock_guard<std::mutex> guardLock(batchLock);
    if (!batches[endb % batches.size()].active)
    {
        ret = true;
        retBatch = batches[endb % batches.size()];
        endb++;
    }
    
    return ret;
}
    
void ParticleSystemDrawable::updateRenderer(SceneRenderer *renderer)
{
    if (usingContinuousRender)
        renderer->addContinuousRenderRequest(getId());
}
    
}
