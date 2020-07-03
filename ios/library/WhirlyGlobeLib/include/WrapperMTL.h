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

namespace WhirlyKit
{

class RenderSetupInfoMTL;

/// Copy one of our matrices into Metal format
void CopyIntoMtlFloat4x4(simd::float4x4 &dest,const Eigen::Matrix4f &src);
void CopyIntoMtlFloat4x4(simd::float4x4 &dest,const Eigen::Matrix4d &src);
void CopyIntoMtlDouble4x4(simd::double4x4 &dest,const Eigen::Matrix4d &src);

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
    
    id<MTLHeap> heap;      // Set if this is in a heap
    id<MTLBuffer> buffer;  // Buffer reference
    int offset;            // Offset within the buffer
};
typedef std::shared_ptr<BufferEntryMTL> BufferEntryMTLRef;

// Used to construct unified buffers for drawables (or whatever)
class BufferBuilderMTL
{
public:
    BufferBuilderMTL(RenderSetupInfoMTL *setupInfo);
    
    // Add the given data to the current buffer construction
    // Will take strides into account so this can be directly referenced within a buffer
    BufferEntryMTLRef addData(const void *data,size_t size);
    
    // Reserve the given space for use by a buffer
    BufferEntryMTLRef reserveData(size_t size);
    
    // Construct the buffer from the data we got
    BufferEntryMTLRef buildBuffer();
public:
    RenderSetupInfoMTL *setupInfo;
    std::vector<BufferEntryMTLRef> bufferRefs;
    NSMutableData *data;
};

// Resources we need for a given render (buffers, textures, etc..)
class ResourceRefsMTL {
public:
    void addEntry(BufferEntryMTLRef entry);
    void addBuffer(id<MTLBuffer> buffer);
    void addTexture(id<MTLTexture> texture);
    void addTextures(const std::vector< id<MTLTexture> > &textures);
    
    void addResources(ResourceRefsMTL &other);
    
    // Wire up the resources listed
    void use(id<MTLRenderCommandEncoder> cmdEncode);
    
    // Drop our references to all the various resources
    void clear();
    
protected:
    std::set< id<MTLHeap> > heaps;
    std::set< id<MTLBuffer> > buffers;
    std::set< id<MTLTexture> > textures;
    
    // We're just hanging on to these till the end of the frame
    std::set< id<MTLBuffer> > buffersToHold;
};
typedef std::shared_ptr<ResourceRefsMTL> ResourceRefsMTLRef;

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
    static bool UseHeaps;
    
    // Allocate a buffer of the given type and size
    // It may be an entry in a heap, it might not.
    // The BufferEntryMTLRef will track it
    BufferEntryMTLRef allocateBuffer(HeapType,size_t size);
    
    // This version copies data into the buffer
    BufferEntryMTLRef allocateBuffer(HeapType,const void *data,size_t size);

protected:
    id<MTLHeap> findHeap(HeapType heapType,size_t &size);

    class HeapGroup
    {
    public:
        std::vector<id<MTLHeap> > heaps;
    };

    id<MTLDevice> mtlDevice;
    HeapGroup heapGroups[MaxType];
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
    BufferEntryMTLRef uniformBuff;
    BufferEntryMTLRef lightingBuff;    
};

    
}
