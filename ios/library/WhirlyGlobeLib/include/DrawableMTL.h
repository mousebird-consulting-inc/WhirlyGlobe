/*
*  DrawableMTL.h
*  WhirlyGlobeLib
*
*  Created by Steve Gifford on 9/30/19.
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

#import "Drawable.h"
#import "SceneRendererMTL.h"

namespace WhirlyKit
{

// Block of data to be passed into a given buffer ID
// We do this rather than setting individual uniforms
class UniformBlockMTL
{
public:
    int bufferID;
    RawDataRef blockData;
};

/**
   Used to track the contents of an argument buffer.
 Includes the buffers we set up to track things.
 */
class ArgBuffContentsMTL {
public:
    // Set up the buffers corresponding to the various entries
    ArgBuffContentsMTL(id<MTLDevice> mtlDevice,
                       RenderSetupInfoMTL *setupInfoMTL,
                       id<MTLFunction> func,
                       int bufferArgIdx,
                       BufferBuilderMTL &buffBuild);
    
    // Check if this is just empty
    bool isEmpty();
        
    // Create empty buffers for the various entries we don't have yet
    void createBuffers(id<MTLDevice> mtlDevice,BufferBuilderMTL &buffBuild);
        
    // True if this argument buffer has the given entry
    bool hasEntry(int entryID);
    
    // True if a constant of the given name exists
    bool hasConstant(const std::string &);
    
    // Make a new buffer, ready to copy arguments into
    void startEncoding(id<MTLDevice> mtlDevice);
    
    // Copy the given entry into the current buffer
    void updateEntry(id<MTLDevice> mtlDevice,
                     id<MTLBlitCommandEncoder> blitEncode,
                     int entryID,
                     void *rawData,size_t size);
    
    // Finished copying arguments into the temp buffer, schedule it to be copied over to the real one
    void endEncoding(id<MTLDevice> mtlDevice,id<MTLBlitCommandEncoder> blitEncode);
            
    // Add the resources we're using to the list
    void addResources(ResourceRefsMTL &resources);
    
    BufferEntryMTL &getBuffer() { return buff; }

    // False if this failed to set up correctly
    bool isValid();
    
protected:
    bool valid;
    bool isSetup;
    RenderSetupInfoMTL *setupInfoMTL;
    
    // Single entry (for a buffer) in the argument buffer
    struct Entry {
        int entryID;
        std::string name;
        
        Entry() { }
        Entry(int entryId) : entryID(entryId) {}
    };
    typedef std::shared_ptr<Entry> EntryRef;

    // Buffer that contains the argument buffer
    BufferEntryMTL buff;
    // Buffer we update into before we copy it over
    BufferEntryMTL tmpBuff;
    
    // Used to encode everything initially and then textures later
    id<MTLArgumentEncoder> encode;
    
    // Individual entries (by ID) in the argument buffer
    std::map<int,EntryRef> entries;
    
    // Indices of the various constants
    std::set<std::string> constants;
};
typedef std::shared_ptr<ArgBuffContentsMTL> ArgBuffContentsMTLRef;

// Assembles the RegularTextures structure for Metal shaders
class ArgBuffRegularTexturesMTL
{
public:
    ArgBuffRegularTexturesMTL(id<MTLDevice> mtlDevice,
                              RenderSetupInfoMTL *setupInfoMTL,
                              id<MTLFunction> mtlFunction,
                              int bufferArgIdx,
                              BufferBuilderMTL &buildBuff);

    // Add a texture to encode
    void addTexture(const Point2f &offset,const Point2f &scale,id<MTLTexture> tex);

    // Encode into a new buffer and schedule an update using
    // Also clears out contents for next encoding pass
    void updateBuffer(id<MTLDevice> mtlDevice,RenderSetupInfoMTL *setupInfoMTL,id<MTLBlitCommandEncoder> bltEncode);
    
    // Size of the texture buffer (fixed)
    size_t encodedLength();

    // List the buffers involved (so they can be tracked)
    void addResources(ResourceRefsMTL &resources);
    
    BufferEntryMTL &getBuffer() { return buffer; }

protected:
    id<MTLArgumentEncoder> encode;
    size_t size;  // Set after encode
    std::vector<Point2f> offsets;
    std::vector<Point2f> scales;
    std::vector<id<MTLTexture> > texs;
    BufferEntryMTL srcBuffer;
    BufferEntryMTL buffer;
};
typedef std::shared_ptr<ArgBuffRegularTexturesMTL> ArgBuffRegularTexturesMTLRef;

/**
    Metal version of drawable doesn't draw, so much as encode.
 */
class DrawableMTL : virtual public Drawable
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
        
    // An all-purpose pre-render that sets up textures, uniforms and such in preparation for rendering
    // Also adds to the list of resources being used by this drawable
    virtual bool preProcess(SceneRendererMTL *sceneRender,
                    id<MTLCommandBuffer> cmdBuff,
                    id<MTLBlitCommandEncoder> bltEncode,
                    SceneMTL *scene) = 0;
    
    /// List all the resources used by the drawable
    virtual void enumerateResources(RendererFrameInfoMTL *frameInfo,ResourceRefsMTL &resources) = 0;

    /// Some drawables have a pre-render phase that uses the GPU for calculation
    virtual void encodeDirectCalculate(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode,Scene *scene) = 0;

    /// Draw directly, once per frame
    virtual void encodeDirect(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode,Scene *scene) = 0;
    
    /// Indirect version of calculate encoding.  Called only when things change enough to re-encode.
    API_AVAILABLE(ios(13.0))
    virtual void encodeIndirectCalculate(id<MTLIndirectRenderCommand> cmdEncode,SceneRendererMTL *sceneRender,Scene *scene,RenderTargetMTL *renderTarget) = 0;

    /// Indirect version of regular encoding.  Called only when things change enough to re-encode.
    API_AVAILABLE(ios(13.0))
    virtual void encodeIndirect(id<MTLIndirectRenderCommand> cmdEncode,SceneRendererMTL *sceneRender,Scene *scene,RenderTargetMTL *renderTarget) = 0;
};

}
