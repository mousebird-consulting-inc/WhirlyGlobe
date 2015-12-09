/*
 *  ParticleSystemDrawable.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/28/15.
 *  Copyright 2011-2015 mousebird consulting
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
#import "CoordSystem.h"

namespace WhirlyKit
{
    
// Shader name
#define kParticleSystemShaderName "Default Part Sys (Point)"
    
// Maximum size of particle buffers (8MB)
#define kMaxParticleMemory (8*1024*1024)

// Build the particle system default shader
OpenGLES2Program *BuildParticleSystemProgram();
    
// Low level drawable used to manage particle systems
class ParticleSystemDrawable : public Drawable
{
public:
    // A group of attribute data passed in at once
    class AttributeData
    {
    public:
        std::string name;
        const void *data;
    };
    
    ParticleSystemDrawable(const std::string &name,const std::vector<SingleVertexAttributeInfo> &vertAttrs,int numTotalPoints,int batchSize,bool useRectangles,bool useInstancing);
    virtual ~ParticleSystemDrawable();
    
    /// No bounding box, since these change constantly
    Mbr getLocalMbr() const { return Mbr(); }

    /// No offset matrix (at the moment)
    const Eigen::Matrix4d *getMatrix() const { return NULL; }

    /// Draw priority for ordering
    unsigned int getDrawPriority() const { return drawPriority; }
    void setDrawPriority(int newPriority) { drawPriority = newPriority; }

    /// Program to use for rendering
    virtual SimpleIdentity getProgram() const { return programId; }
    /// Set the shader program.  Empty (default) by default
    virtual void setProgram(SimpleIdentity newProgId) { programId = newProgId; }

    /// Whether it's currently displaying
    bool isOn(WhirlyKitRendererFrameInfo *frameInfo) const;
    /// True to turn it on, false to turn it off
    void setOnOff(bool onOff) { enable = onOff; }
    
    /// Set the base time
    void setBaseTime(NSTimeInterval inBaseTime) { baseTime = inBaseTime; }
    
    /// Set the point size
    void setPointSize(float inPointSize) { pointSize = inPointSize; }
    
    /// Set the lifetime
    void setLifetime(NSTimeInterval inLifetime) { lifetime = inLifetime; }
    NSTimeInterval getLifetime() { return lifetime; }
    
    /// Set all the textures at once
    virtual void setTexIDs(const std::vector<SimpleIdentity> &inTexIDs) { texIDs = inTexIDs; }
    
    /// Create our buffers in GL
    void setupGL(WhirlyKitGLSetupInfo *setupInfo,OpenGLMemManager *memManager);
    
    /// Destroy GL buffers
    void teardownGL(OpenGLMemManager *memManager);
    
    /// Called on the rendering thread to draw
    void draw(WhirlyKitRendererFrameInfo *frameInfo,Scene *scene);
    
    /// Just points for now
    GLenum getType() const { return GL_POINTS; }
    
    /// Not using this mechanism
    bool hasAlpha(WhirlyKitRendererFrameInfo *frameInfo) const { return false; }
    
    /// Don't need to update the renderer particularly
    void updateRenderer(WhirlyKitSceneRendererES *renderer);

    /// If set, we want to use the z buffer
    bool getRequestZBuffer() const { return requestZBuffer; }
    void setRequestZBuffer(bool enable) { requestZBuffer = enable; }
    
    /// If set, we want to write to the z buffer
    bool getWriteZbuffer() const { return writeZBuffer; }
    void setWriteZbuffer(bool enable) { writeZBuffer = enable; }
    
    // Represents a single batch of data
    class Batch
    {
    public:
        unsigned int batchID;
        unsigned int offset,len;
        bool active;
        NSTimeInterval startTime;
    };
    
    /// Add the vertex data (all of it) at once
    void addAttributeData(const std::vector<AttributeData> &attrData,const Batch &batch);
    
    /// Look for an empty batch to reuse
    bool findEmptyBatch(Batch &retBatch);
    
    /// Invalidate old batches
    void updateBatches(NSTimeInterval now);

protected:
    bool enable;
    int numTotalPoints,batchSize;
    int vertexSize;
    std::vector<SingleVertexAttributeInfo> vertAttrs;
    SimpleIdentity programId;
    int drawPriority;
    float pointSize;
    NSTimeInterval lifetime;
    bool requestZBuffer,writeZBuffer;
    float minVis,maxVis,minVisibleFadeBand,maxVisibleFadeBand;
    GLuint pointBuffer,rectBuffer;
    std::vector<SimpleIdentity> texIDs;
    bool useRectangles,useInstancing;
    NSTimeInterval baseTime;

    // The vertex attributes we're representing in the buffers
    std::vector<VertexAttribute> vertexAttributes;
    
    // Chunk of a buffer to render
    typedef struct
    {
        int bufferStart;
        int numVertices;
    } BufferChunk;
    
    void updateChunks();
    
    // Chunks we use for rendering
    pthread_mutex_t batchLock;
    int startb,endb;
    std::vector<Batch> batches;
    bool chunksDirty;
    std::vector<BufferChunk> chunks;
};

}
