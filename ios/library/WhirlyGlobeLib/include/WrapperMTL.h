/*
 *  WrapperMTL.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/16/19.
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
 *
 */

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import "ChangeRequest.h"
#import "baseInfo.h"
#import "DefaultShadersMTL.h"
#import <unordered_set>

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
    bool operator == (const BufferEntryMTL &that);
    void clear();
    
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
    
    void addEntry(const BufferEntryMTL &entry);
    void addBuffer(id<MTLBuffer> buffer);
    void addTexture(const TextureEntryMTL &texture);
    void addTextures(const std::vector<TextureEntryMTL> &textures);

    void addResources(const ResourceRefsMTL &other);
    
    // Wire up the resources listed
    void use(id<MTLRenderCommandEncoder> cmdEncode);
    
    // Drop our references to all the various resources
    void clear();
    
protected:
    NSMutableSet< id<MTLHeap> > *heaps;
    NSMutableSet< id<MTLBuffer> > *buffers;
    NSMutableSet< id<MTLTexture> > *textures;
    
    bool trackHolds;
    
    // We're just hanging on to these till the end of the frame
    NSMutableSet< id<MTLBuffer> > *buffersToHold;
    NSMutableSet< id<MTLTexture> > *texturesToHold;
};
typedef std::shared_ptr<ResourceRefsMTL> ResourceRefsMTLRef;

class DrawGroupMTL;
typedef std::shared_ptr<DrawGroupMTL> DrawGroupMTLRef;

// Used to track resources that we're tearing down
// We need to hold on to them until after the current frame is done
class RenderTeardownInfoMTL : public RenderTeardownInfo {
public:
    RenderTeardownInfoMTL();
    
    void clear();

    // Hold on to a draw group until releasing it later
    void releaseDrawGroups(SceneRenderer *renderer,std::vector<DrawGroupMTLRef> &ref);

    // Either destroy the texture now or hold on to it for destruction shortly
    void destroyTexture(SceneRenderer *renderer,const TextureBaseRef &tex) override;

    // Either destroy the drawable now or hold on to it for destruction shortly
    void destroyDrawable(SceneRenderer *renderer,const DrawableRef &draw) override;

public:
    ResourceRefsMTLRef resources;

protected:
    // Hold these objects and release them on another thread
    std::vector<DrawGroupMTLRef> drawGroups;
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
    
    // Update the amount of space on the various heaps
    void updateHeaps();
    
    // Allocate a buffer of the given type and size
    // It may be an entry in a heap, it might not.
    // The BufferEntryMTLRef will track it
    BufferEntryMTL allocateBuffer(HeapType,size_t size);
    
    // This version copies data into the buffer
    BufferEntryMTL allocateBuffer(HeapType,const void *data,size_t size);
    
    // Allocate a texture with the given descriptor off of a heap (or not)
    TextureEntryMTL newTextureWithDescriptor(MTLTextureDescriptor *desc,size_t size);

protected:
    // Info about a single heap
    struct HeapInfo
    {
        size_t maxAvailSize;
        id<MTLHeap> heap;
    };
    typedef std::shared_ptr<HeapInfo> HeapInfoRef;
    
    typedef struct _HeapGroupSorter {
        bool operator() (const HeapInfoRef &a, const HeapInfoRef &b) const {
            if (a->maxAvailSize == b->maxAvailSize)
                return a->heap < b->heap;
            return a->maxAvailSize < b->maxAvailSize;
        }
    } HeapGroupSorter;
    using HeapSet = std::set<HeapInfoRef, HeapGroupSorter>;
    
    // Group of heaps sorted by max available size
    struct HeapGroup
    {
        HeapSet heaps;
    };
    
    HeapInfoRef allocateHeap(unsigned size, unsigned minSize, MTLStorageMode mode);
    HeapInfoRef findHeap(HeapType heapType,size_t &size,id<MTLHeap> prevHeap = nil);
    HeapInfoRef findHeap(HeapSet &heapSet,size_t &size,id<MTLHeap> prevHeap = nil);
    HeapInfoRef findTextureHeap(MTLTextureDescriptor *desc,size_t size,id<MTLHeap> prevHeap = nil);

    std::mutex lock;
    std::mutex texLock;
    id<MTLDevice> mtlDevice;
    HeapGroup heapGroups[MaxType];
    HeapGroup texGroups;

    // Keep Metal allocations aligned to this
    size_t memAlign;
    
    static constexpr size_t MB = 1024 * 1024;
};

#define MaxViewWrap 3

/// Passed around to various init and teardown routines
struct RenderSetupInfoMTL : public RenderSetupInfo
{
    RenderSetupInfoMTL(id<MTLDevice> mtlDevice,id<MTLLibrary> mtlLibrary);
    
    id<MTLDevice> mtlDevice;

    HeapManagerMTL heapManage;
    
    // Keep Metal allocations aligned to this
    size_t memAlign;

    // Buffers created for shared uniforms.
    // Wired into the various drawables individually
    BufferEntryMTL uniformBuff[MaxViewWrap];   // We can have three different instances of all geometry (for view wrapping)
    BufferEntryMTL lightingBuff;
};

/// Convert  a float expression into its Metal version
void FloatExpressionToMtl(const FloatExpressionInfoRef &srcInfo,
                          WhirlyKitShader::FloatExp &destExp);

/// Convert a color expression into its Metal version
void ColorExpressionToMtl(const ColorExpressionInfoRef &srcInfo,
                          WhirlyKitShader::ColorExp &destExp);

}
