/*  ParticleSystemDrawable.h
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

#import "BasicDrawable.h"
#import "Program.h"
#import "CoordSystem.h"

namespace WhirlyKit
{

// Low level drawable used to manage particle systems
struct ParticleSystemDrawable : virtual public Drawable
{
    friend class ParticleSystemDrawableBuilder;

    // A group of attribute data passed in at once
    struct AttributeData
    {
        std::string name;
        const void *data = nullptr;
    };
    
    ParticleSystemDrawable(const std::string &name);
    virtual ~ParticleSystemDrawable() = default;

    /// Whether it's currently displaying
    bool isOn(RendererFrameInfo *frameInfo) const { return enable; }
    void setOnOff(bool onOff) { enable = onOff; }

    /// No bounding box, since these change constantly
    Mbr getLocalMbr() const { return {}; }

    /// No offset matrix (at the moment)
    const Eigen::Matrix4d *getMatrix() const { return nullptr; }

    /// Draw order
    int64_t getDrawOrder() const { return drawOrder; }
    void setDrawOrder(int64_t newOrder) { drawOrder = newOrder; }
    
    /// Draw priority for ordering
    unsigned int getDrawPriority() const { return drawPriority; }
    void setDrawPriority(int newPriority) { drawPriority = newPriority; }

    /// If set, we want to use the z buffer
    bool getRequestZBuffer() const { return requestZBuffer; }
    void setRequestZBuffer(bool in) { requestZBuffer = in; }
    
    /// If set, we want to write to the z buffer
    bool getWriteZbuffer() const { return writeZBuffer; }
    void setWriteZbuffer(bool in) { writeZBuffer = in; }

    // If set, we'll render this data where directed
    void setRenderTarget(SimpleIdentity newRenderTarget) { renderTargetID = newRenderTarget; }
    SimpleIdentity getRenderTarget() const { return renderTargetID; }
    
    /// Set all the textures at once
    virtual void setTexIDs(const std::vector<SimpleIdentity> &inTexIDs) { texIDs = inTexIDs; }

    /// Program to use for pre-render calculations
    virtual SimpleIdentity getCalculationProgram() const { return calculateProgramId; }
    virtual void setCalculationProgram(SimpleIdentity newProgId) { calculateProgramId = newProgId; }

    /// Program to use for rendering
    virtual SimpleIdentity getProgram() const { return renderProgramId; }
    virtual void setProgram(SimpleIdentity newProgId) { renderProgramId = newProgId; }
    
    /// Set the base time
    void setBaseTime(TimeInterval inBaseTime) { baseTime = inBaseTime; }
    
    /// Set the point size
    //void setPointSize(float inPointSize) { pointSize = inPointSize; }
    
    /// Set the lifetime
    void setLifetime(TimeInterval inLifetime) { lifetime = inLifetime; }
    TimeInterval getLifetime() const { return lifetime; }
    
    /// Set whether we're doing continuous renders (the default)
    void setContinuousUpdate(bool newVal) { usingContinuousRender = newVal; }
        
    /// Set a block of uniforms (Metal only, at the moment)
    virtual void setUniBlock(const BasicDrawable::UniformBlock &uniBlock);
    
    /// Don't need to update the renderer particularly
    void updateRenderer(SceneRenderer *renderer);
    
    // Represents a single batch of data
    struct Batch
    {
        TimeInterval startTime;
        unsigned int batchID;
        unsigned int offset;
        unsigned int len;
        bool active;
    };
    
    /// Add a batch for rendering later
    /// OpenGL ES wants individual attributes.  Metal just a blob of data.
    virtual void addAttributeData(const RenderSetupInfo *setupInfo,const std::vector<AttributeData> &attrData,const Batch &batch) { };
    virtual void addAttributeData(const RenderSetupInfo *setupInfo,const RawDataRef &data,const Batch &batch) { };

    /// Called once to set up batches
    void setupBatches();
    
    /// Look for an empty batch to reuse
    bool findEmptyBatch(Batch &retBatch);
    
    /// Invalidate old batches
    void updateBatches(TimeInterval now);

protected:
    bool enable = true;
    int numTotalPoints = 0;
    int batchSize = 0;
    int vertexSize = 0;
    SimpleIdentity calculateProgramId = EmptyIdentity;
    SimpleIdentity renderProgramId = EmptyIdentity;
    int64_t drawOrder = BaseInfo::DrawOrderTiles;
    int drawPriority = 0;
    TimeInterval lifetime = 0.0;
    bool requestZBuffer = false;
    bool writeZBuffer = false;
    float minVis = DrawVisibleInvalid;
    float maxVis = DrawVisibleInvalid;
    //float minVisibleFadeBand = DrawVisibleInvalid;
    //float maxVisibleFadeBand = DrawVisibleInvalid;
    int activeVaryBuffer = 0;  // 0 or 1
    std::vector<SimpleIdentity> texIDs;
    bool useRectangles = true;
    bool useInstancing = true;
    TimeInterval baseTime = 0.0;
    bool usingContinuousRender = true;
    SimpleIdentity renderTargetID = EmptyIdentity;

    // Uniforms to be passed into a shader (just Metal for now)
    std::vector<BasicDrawable::UniformBlock> uniBlocks;

    // Chunk of a buffer to render
    typedef struct
    {
        int bufferStart;
        int vertexStart;
        int numVertices;
    } BufferChunk;
    
    TimeInterval lastUpdateTime = 0.0;
    void updateChunks();
    
    // Chunks we use for rendering
    std::mutex batchLock;
    int startb = 0;
    int endb = 0;
    std::vector<Batch> batches;
    bool chunksDirty = true;
    std::vector<BufferChunk> chunks;
};

/// Reference counted version of ParticleSystemDrawable
typedef std::shared_ptr<ParticleSystemDrawable> ParticleSystemDrawableRef;

}
