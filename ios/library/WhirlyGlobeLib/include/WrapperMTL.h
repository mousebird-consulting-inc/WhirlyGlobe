/*
 *  WrapperMTL.h
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

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import "ChangeRequest.h"
#import "baseInfo.h"
#import "DefaultShadersMTL.h"

namespace WhirlyKit
{

class RenderSetupInfoMTL;

/// Copy one of our matrices into Metal format
void CopyIntoMtlFloat4x4(simd::float4x4 &dest,const Eigen::Matrix4f &src);
void CopyIntoMtlFloat4x4(simd::float4x4 &dest,const Eigen::Matrix4d &src);
void CopyIntoMtlDouble4x4(simd::double4x4 &dest,const Eigen::Matrix4d &src);

/// This version works like CopyIntoMtlDouble4x4 and then puts the difference into destDiff
void CopyIntoMtlFloat4x4Pair(simd::float4x4 &dest,simd::float4x4 &destDiff,const Eigen::Matrix4d &src);

/// Copy one of our points into Metal form
void CopyIntoMtlFloat3(simd::float3 &dest,const Point3d &src);
void CopyIntoMtlFloat3(simd::float3 &dest,const Point3f &src);
void CopyIntoMtlFloat2(simd::float2 &dest,const Point2f &src);

/// Copy one of our 4D points into Metal form
void CopyIntoMtlFloat4(simd::float4 &dest,const Eigen::Vector4f &src);
void CopyIntoMtlFloat4(simd::float4 &dest,const float vals[4]);

/// Entry in a larger buffer structure
class BufferEntryMTL {
public:
    BufferEntryMTL();
    BufferEntryMTL(const BufferEntryMTL &that);
    bool operator == (const BufferEntryMTL &that);
    void clear();
    BufferEntryMTL & operator = (const BufferEntryMTL &that);
    
    bool valid;            // Manipulating functions will set this
    id<MTLHeap> heap;      // Set if this is in a heap
    id<MTLBuffer> buffer;  // Buffer reference
    int offset;            // Offset within the buffer
};

/// Description of what and where a texture is
class TextureEntryMTL {
public:
    TextureEntryMTL();
    
    id<MTLHeap> heap;    // Set if this is in a heap
    id<MTLTexture> tex;  // The texture itself
};

// Used to construct unified buffers for drawables (or whatever)
class BufferBuilderMTL
{
public:
    BufferBuilderMTL(RenderSetupInfoMTL *setupInfo);
    
    // Add the given data to the current buffer construction
    // Will take strides into account so this can be directly referenced within a buffer
    void addData(const void *data,size_t size,BufferEntryMTL *buffer);
    
    // Reserve the given space for use by a buffer
    void reserveData(size_t size,BufferEntryMTL *buffer);
    
    // Construct the buffer from the data we got
    BufferEntryMTL buildBuffer();
public:
    RenderSetupInfoMTL *setupInfo;
    std::vector<BufferEntryMTL *> bufferRefs;
    NSMutableData *data;
};

// Resources we need for a given render (buffers, textures, etc..)
class ResourceRefsMTL {
public:
    // Construct either track all buffers, or just track what we need to use()
    ResourceRefsMTL(bool trackHolds=false);
    
    void addEntry(BufferEntryMTL &entry);
    void addBuffer(id<MTLBuffer> buffer);
    void addTexture(TextureEntryMTL &texture);
    void addTextures(const std::vector<TextureEntryMTL> &textures);

    void addResources(ResourceRefsMTL &other);
    
    // Wire up the resources listed
    void use(id<MTLRenderCommandEncoder> cmdEncode);
    
    // Drop our references to all the various resources
    void clear();
    
protected:
    std::set< id<MTLHeap> > heaps;
    std::set< id<MTLBuffer> > buffers;
    std::set< id<MTLTexture> > textures;
    
    bool trackHolds;
    
    // We're just hanging on to these till the end of the frame
    std::set< id<MTLBuffer> > buffersToHold;
    std::set< id<MTLTexture> > texturesToHold;
};
typedef std::shared_ptr<ResourceRefsMTL> ResourceRefsMTLRef;

// Used to track resources that we're tearing down
// We need to hold on to them until after the current frame is done
class RenderTeardownInfoMTL : public RenderTeardownInfo {
public:
    RenderTeardownInfoMTL();
    
    void clear();

    // Either destroy the texture now or hold on to it for destruction shortly
    void destroyTexture(SceneRenderer *renderer,const TextureBaseRef &tex) override;

    // Either destroy the drawable now or hold on to it for destruction shortly
    void destroyDrawable(SceneRenderer *renderer,const DrawableRef &draw) override;

public:
    ResourceRefsMTLRef resources;

protected:
    // Hold these objects and release them on another thread
    std::vector<DrawableRef> drawables;
    std::vector<TextureBaseRef> textures;
};
typedef std::shared_ptr<RenderTeardownInfoMTL> RenderTeardownInfoMTLRef;

/**
   Used to manage the various MTLHeaps we depend on.
 Heaps are sorted by type and usage.
 */
class HeapManagerMTL
{
public:
    HeapManagerMTL(id<MTLDevice> mtlDevice);
    
    // Types of heaps for sorting
    typedef enum {Drawable,Other,MaxType} HeapType;
    
    // If false, we'll allocate individual buffers
    static const bool UseHeaps;
    
    // Allocate a buffer of the given type and size
    // It may be an entry in a heap, it might not.
    // The BufferEntryMTLRef will track it
    BufferEntryMTL allocateBuffer(HeapType,size_t size);
    
    // This version copies data into the buffer
    BufferEntryMTL allocateBuffer(HeapType,const void *data,size_t size);
    
    // Allocate a texture with the given descriptor off of a heap (or not)
    TextureEntryMTL newTextureWithDescriptor(MTLTextureDescriptor *desc,size_t size);

protected:
    id<MTLHeap> findHeap(HeapType heapType,size_t &size);
    id<MTLHeap> findTextureHeap(MTLTextureDescriptor *desc,size_t size);

    class HeapGroup
    {
    public:
        std::vector<id<MTLHeap> > heaps;
    };

    std::mutex lock;
    std::mutex texLock;
    id<MTLDevice> mtlDevice;
    HeapGroup heapGroups[MaxType];
    HeapGroup texGroups;
};

/// Passed around to various init and teardown routines
class RenderSetupInfoMTL : public RenderSetupInfo
{
public:
    RenderSetupInfoMTL(id<MTLDevice> mtlDevice,id<MTLLibrary> mtlLibrary);
    
    id<MTLDevice> mtlDevice;
    
    HeapManagerMTL heapManage;
    
    // Keep Metal allocations aligned to this
    size_t memAlign;

    // Buffers created for shared uniforms.
    // Wired into the various drawables individually
    BufferEntryMTL uniformBuff;
    BufferEntryMTL lightingBuff;
};

/// Convert  a float expression into its Metal version
void FloatExpressionToMtl(FloatExpressionInfoRef srcInfo,WhirlyKitShader::FloatExp &destExp);

/// Convert a color expression into its Metal version
void ColorExpressionToMtl(ColorExpressionInfoRef srcInfo,WhirlyKitShader::ColorExp &destExp);

}
