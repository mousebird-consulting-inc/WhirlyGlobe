/*
 *  ParticleSystemDrawable.h
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

#import "BasicDrawable.h"
#import "Program.h"
#import "CoordSystem.h"

namespace WhirlyKit
{

// Low level drawable used to manage particle systems
class ParticleSystemDrawable : virtual public Drawable
{
friend class ParticleSystemDrawableBuilder;
public:    
    // A group of attribute data passed in at once
    class AttributeData
    {
    public:
        std::string name;
        const void *data;
    };
    
    ParticleSystemDrawable(const std::string &name);
    virtual ~ParticleSystemDrawable();
    
    /// Whether it's currently displaying
    bool isOn(RendererFrameInfo *frameInfo) const;
    void setOnOff(bool onOff);

    /// No bounding box, since these change constantly
    Mbr getLocalMbr() const;

    /// No offset matrix (at the moment)
    const Eigen::Matrix4d *getMatrix() const;
    
    /// Draw priority for ordering
    unsigned int getDrawPriority() const;
    void setDrawPriority(int newPriority);

    /// If set, we want to use the z buffer
    bool getRequestZBuffer() const;
    void setRequestZBuffer(bool enable);
    
    /// If set, we want to write to the z buffer
    bool getWriteZbuffer() const;
    void setWriteZbuffer(bool enable);

    // If set, we'll render this data where directed
    void setRenderTarget(SimpleIdentity newRenderTarget);
    SimpleIdentity getRenderTarget() const;
    
    /// Set all the textures at once
    virtual void setTexIDs(const std::vector<SimpleIdentity> &inTexIDs);

    /// Program to use for pre-render calculations
    virtual SimpleIdentity getCalculationProgram() const;
    virtual void setCalculationProgram(SimpleIdentity newProgId);

    /// Program to use for rendering
    virtual SimpleIdentity getProgram() const;
    virtual void setProgram(SimpleIdentity newProgId);
    
    /// Set the base time
    void setBaseTime(TimeInterval inBaseTime);
    
    /// Set the point size
    void setPointSize(float inPointSize);
    
    /// Set the lifetime
    void setLifetime(TimeInterval inLifetime);
    TimeInterval getLifetime();
    
    /// Set whether we're doing continuous renders (the default)
    void setContinuousUpdate(bool newVal);
        
    /// Set a block of uniforms (Metal only, at the moment)
    virtual void setUniBlock(const BasicDrawable::UniformBlock &uniBlock);
    
    /// Don't need to update the renderer particularly
    void updateRenderer(SceneRenderer *renderer);
    
    // Represents a single batch of data
    class Batch
    {
    public:
        unsigned int batchID;
        unsigned int offset,len;
        bool active;
        TimeInterval startTime;
    };
    
    /// Add a batch for rendering later
    /// OpenGL ES wants individual attributes.  Metal just a blob of data.
    virtual void addAttributeData(const RenderSetupInfo *setupInfo,const std::vector<AttributeData> &attrData,const Batch &batch) { };
    virtual void addAttributeData(const RenderSetupInfo *setupInfo,const RawDataRef &data,const Batch &batch) { };

    /// Called once to set up batches
    void setupBaches();
    
    /// Look for an empty batch to reuse
    bool findEmptyBatch(Batch &retBatch);
    
    /// Invalidate old batches
    void updateBatches(TimeInterval now);

protected:
    bool enable;
    int numTotalPoints,batchSize;
    int vertexSize;
    SimpleIdentity calculateProgramId;
    SimpleIdentity renderProgramId;
    int drawPriority;
    float pointSize;
    TimeInterval lifetime;
    bool requestZBuffer,writeZBuffer;
    float minVis,maxVis,minVisibleFadeBand,maxVisibleFadeBand;
    int activeVaryBuffer;  // 0 or 1
    std::vector<SimpleIdentity> texIDs;
    bool useRectangles,useInstancing;
    TimeInterval baseTime;
    bool usingContinuousRender;
    SimpleIdentity renderTargetID;

    // The vertex attributes we're representing in the buffers
    std::vector<VertexAttribute> vertexAttributes;
    // Uniforms to be passed into a shader (just Metal for now)
    std::vector<BasicDrawable::UniformBlock> uniBlocks;

    // Chunk of a buffer to render
    typedef struct
    {
        int bufferStart;
        int vertexStart;
        int numVertices;
    } BufferChunk;
    
    TimeInterval lastUpdateTime;
    void updateChunks();
    
    // Chunks we use for rendering
    std::mutex batchLock;
    int startb,endb;
    std::vector<Batch> batches;
    bool chunksDirty;
    std::vector<BufferChunk> chunks;
};

/// Reference counted version of ParticleSystemDrawable
typedef std::shared_ptr<ParticleSystemDrawable> ParticleSystemDrawableRef;

}
