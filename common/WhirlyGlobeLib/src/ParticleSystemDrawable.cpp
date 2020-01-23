/*
 *  ParticleSystemDrawable.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/28/15.
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

#import "ParticleSystemDrawable.h"
#import "SceneRenderer.h"
#import "WhirlyKitLog.h"


namespace WhirlyKit
{

ParticleSystemDrawable::ParticleSystemDrawable(const std::string &name)
    : Drawable(name), enable(true), numTotalPoints(0), batchSize(0), vertexSize(0),
    calculateProgramId(EmptyIdentity), renderProgramId(EmptyIdentity), drawPriority(0),
    requestZBuffer(false), writeZBuffer(false),
    maxVis(DrawVisibleInvalid), useRectangles(true), useInstancing(true), baseTime(0.0),
    startb(0.0), endb(0.0), chunks(true), usingContinuousRender(true), renderTargetID(EmptyIdentity),
    lastUpdateTime(0.0), activeVaryBuffer(0)
{
}
    
ParticleSystemDrawable::~ParticleSystemDrawable()
{
}
    
bool ParticleSystemDrawable::isOn(RendererFrameInfo *frameInfo) const
{
    if (!enable)
        return false;
    
    return true;
}
    
void ParticleSystemDrawable::setOnOff(bool onOff)
    { enable = onOff; }
    
Mbr ParticleSystemDrawable::getLocalMbr() const
    { return Mbr(); }

const Eigen::Matrix4d *ParticleSystemDrawable::getMatrix() const
    { return NULL; }

unsigned int ParticleSystemDrawable::getDrawPriority() const
    { return drawPriority; }
    
void ParticleSystemDrawable::setDrawPriority(int newPriority)
    { drawPriority = newPriority; }

bool ParticleSystemDrawable::getRequestZBuffer() const
    { return requestZBuffer; }
    
void ParticleSystemDrawable::setRequestZBuffer(bool enable)
    { requestZBuffer = enable; }
    
bool ParticleSystemDrawable::getWriteZbuffer() const
    { return writeZBuffer; }
    
void ParticleSystemDrawable::setWriteZbuffer(bool enable)
    { writeZBuffer = enable; }

void ParticleSystemDrawable::setRenderTarget(SimpleIdentity newRenderTarget)
    { renderTargetID = newRenderTarget; }
    
SimpleIdentity ParticleSystemDrawable::getRenderTarget() const
    { return renderTargetID; }
    
void ParticleSystemDrawable::setTexIDs(const std::vector<SimpleIdentity> &inTexIDs)
    { texIDs = inTexIDs; }

SimpleIdentity ParticleSystemDrawable::getCalculationProgram() const
    { return calculateProgramId; }
    
void ParticleSystemDrawable::setCalculationProgram(SimpleIdentity newProgId)
    { calculateProgramId = newProgId; }

SimpleIdentity ParticleSystemDrawable::getProgram() const
    { return renderProgramId; }
    
void ParticleSystemDrawable::setProgram(SimpleIdentity newProgId)
    { renderProgramId = newProgId; }

void ParticleSystemDrawable::setBaseTime(TimeInterval inBaseTime)
    { baseTime = inBaseTime; }

void ParticleSystemDrawable::setPointSize(float inPointSize)
    
    { pointSize = inPointSize; }

void ParticleSystemDrawable::setLifetime(TimeInterval inLifetime)
    { lifetime = inLifetime; }
    
TimeInterval ParticleSystemDrawable::getLifetime()
    { return lifetime; }

void ParticleSystemDrawable::setContinuousUpdate(bool newVal)
    { usingContinuousRender = newVal; }

void ParticleSystemDrawable::setUniBlock(const BasicDrawable::UniformBlock &uniBlock)
{
    for (int ii=0;ii<uniBlocks.size();ii++)
        if (uniBlocks[ii].bufferID == uniBlock.bufferID) {
            uniBlocks[ii] = uniBlock;
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
                    chunk.bufferStart = (start % batches.size()) * batchSize * vertexSize;
                    chunk.vertexStart = (start % batches.size()) * batchSize;
                    chunk.numVertices = (end-start) * batchSize;
                    chunks.push_back(chunk);
                }
            }
            
            start = end;
        } while (start < batches.size());
    }
}
    
void ParticleSystemDrawable::setupBaches()
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
