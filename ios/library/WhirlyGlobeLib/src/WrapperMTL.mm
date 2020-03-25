/*
 *  WrapperMTL.mm
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

#import "WrapperMTL.h"

namespace WhirlyKit
{

void CopyIntoMtlFloat4x4(simd::float4x4 &dest,const Eigen::Matrix4f &src)
{
    for (unsigned int ix=0;ix<4;ix++)
        for (unsigned int iy=0;iy<4;iy++)
            dest.columns[ix][iy] = src(ix*4+iy);
}
    
void CopyIntoMtlFloat4x4(simd::float4x4 &dest,const Eigen::Matrix4d &src)
{
    for (unsigned int ix=0;ix<4;ix++)
        for (unsigned int iy=0;iy<4;iy++)
            dest.columns[ix][iy] = src(ix*4+iy);
}

void CopyIntoMtlDouble4x4(simd::double4x4 &dest,const Eigen::Matrix4d &src)
{
    for (unsigned int ix=0;ix<4;ix++)
        for (unsigned int iy=0;iy<4;iy++)
            dest.columns[ix][iy] = src(ix*4+iy);
}
    
void CopyIntoMtlFloat3(simd::float3 &dest,const Point3d &src)
{
    dest[0] = src.x();
    dest[1] = src.y();
    dest[2] = src.z();
}
    
void CopyIntoMtlFloat3(simd::float3 &dest,const Point3f &src)
{
    dest[0] = src.x();
    dest[1] = src.y();
    dest[2] = src.z();
}
    
void CopyIntoMtlFloat2(simd::float2 &dest,const Point2f &src)
{
    dest[0] = src.x();
    dest[1] = src.y();
}

void CopyIntoMtlFloat4(simd::float4 &dest,const Eigen::Vector4f &src)
{
    dest[0] = src.x();
    dest[1] = src.y();
    dest[2] = src.z();
    dest[3] = src.w();
}
    
void CopyIntoMtlFloat4(simd::float4 &dest,const float vals[4])
{
    dest[0] = vals[0];
    dest[1] = vals[1];
    dest[2] = vals[2];
    dest[3] = vals[3];
}

BufferEntryMTL::BufferEntryMTL()
: heap(nil), buffer(nil), offset(0)
{
}

BufferBuilderMTL::BufferBuilderMTL(RenderSetupInfoMTL *setupInfo)
: data(nil), setupInfo(setupInfo)
{
    data = [NSMutableData new];
}

BufferEntryMTLRef BufferBuilderMTL::addData(const void *inData,size_t size)
{
    if (HeapManagerMTL::UseHeaps) {
        size_t start = [data length];
        [data appendBytes:inData length:size];

        // Keep the allocations on the alignment Metal wants
        size_t extra = size % setupInfo->memAlign;
        if (extra > 0) {
            [data increaseLengthBy:setupInfo->memAlign - extra];
            [data resetBytesInRange:NSMakeRange(start + size, setupInfo->memAlign - extra)];
        }

        BufferEntryMTLRef buffer(new BufferEntryMTL());
        buffer->offset = start;
        bufferRefs.push_back(buffer);
        
        return buffer;
    } else {
        return setupInfo->heapManage.allocateBuffer(HeapManagerMTL::Drawable, inData, size);
    }
}

BufferEntryMTLRef BufferBuilderMTL::reserveData(size_t size)
{
    if (HeapManagerMTL::UseHeaps) {
        // Keep the allocations on the alignment Metal wants
        size_t extra = size % setupInfo->memAlign;
        if (extra > 0) {
            size += setupInfo->memAlign - extra;
        }

        int start = [data length];
        [data increaseLengthBy:size];
        [data resetBytesInRange:NSMakeRange(start, size)];

        BufferEntryMTLRef buffer(new BufferEntryMTL());
        buffer->offset = start;
        bufferRefs.push_back(buffer);

        return buffer;
    } else {
        return setupInfo->heapManage.allocateBuffer(HeapManagerMTL::Drawable, size);
    }
}

BufferEntryMTLRef BufferBuilderMTL::buildBuffer()
{
    if ([data length] == 0)
        return BufferEntryMTLRef();
        
    if (HeapManagerMTL::UseHeaps) {
        BufferEntryMTLRef buffer = setupInfo->heapManage.allocateBuffer(HeapManagerMTL::Drawable, [data mutableBytes],[data length]);
        
        // Update all the buffer references we already created
        for (auto bufRef : bufferRefs) {
            bufRef->heap = buffer->heap;
            bufRef->buffer = buffer->buffer;
        }

        bufferRefs.clear();
        
        return buffer;
    }

    bufferRefs.clear();
    return nil;
}


void ResourceRefsMTL::addEntry(BufferEntryMTLRef entry)
{
    if (!entry)
        return;
    if (entry->heap)
        heaps.insert(entry->heap);
    else if (entry->buffer)
        buffers.insert(entry->buffer);
}

void ResourceRefsMTL::addBuffer(id<MTLBuffer> buffer)
{
    buffers.insert(buffer);
}

void ResourceRefsMTL::addTexture(id<MTLTexture> texture)
{
    textures.insert(texture);
}

void ResourceRefsMTL::addTextures(const std::vector< id<MTLTexture> > &newTextures)
{
    textures.insert(newTextures.begin(),newTextures.end());
}

void ResourceRefsMTL::use(id<MTLRenderCommandEncoder> cmdEncode)
{
    for (id<MTLHeap> heap : heaps)
        [cmdEncode useHeap:heap];
    for (id<MTLBuffer> buff : buffers)
        [cmdEncode useResource:buff usage:MTLResourceUsageRead];
    for (id<MTLTexture> tex : textures)
        [cmdEncode useResource:tex usage:MTLResourceUsageRead];
}

bool HeapManagerMTL::UseHeaps = true;

HeapManagerMTL::HeapManagerMTL(id<MTLDevice> mtlDevice)
: mtlDevice(mtlDevice)
{
}

id<MTLHeap> HeapManagerMTL::findHeap(HeapType heapType,size_t &size)
{
    HeapGroup &heapGroup = heapGroups[heapType];
    for (auto heap : heapGroup.heaps) {
        MTLSizeAndAlign sAlign = [mtlDevice heapBufferSizeAndAlignWithLength:size options:MTLResourceUsageRead];
        size = sAlign.size;
        if ([heap maxAvailableSizeWithAlignment:sAlign.align] >= size)
            return heap;
    }
    MTLHeapDescriptor *heapDesc = [[MTLHeapDescriptor alloc] init];
    heapDesc.size = 16*1024*10214;  // 16MB, at a guess
    // TODO: Don't need this for most things
    heapDesc.storageMode = MTLStorageModeShared;
    id<MTLHeap> newHeap = [mtlDevice newHeapWithDescriptor:heapDesc];
    heapGroup.heaps.push_back(newHeap);
    
    return newHeap;
}

BufferEntryMTLRef HeapManagerMTL::allocateBuffer(HeapType heapType,size_t size)
{
    BufferEntryMTLRef buffer(new BufferEntryMTL());
    if (UseHeaps) {
        buffer->heap = findHeap(heapType,size);
        buffer->buffer = [buffer->heap newBufferWithLength:size options:MTLResourceStorageModeShared];
        buffer->offset = 0;
    } else {
        buffer->buffer = [mtlDevice newBufferWithLength:size options:MTLResourceStorageModeShared];
        buffer->heap = nil;
        buffer->offset = 0;
    }
    
    return buffer;
}

BufferEntryMTLRef HeapManagerMTL::allocateBuffer(HeapType heapType,const void *data,size_t size)
{
    BufferEntryMTLRef buffer(new BufferEntryMTL());
    if (UseHeaps) {
        buffer->heap = findHeap(heapType,size);
        buffer->buffer = [buffer->heap newBufferWithLength:size options:MTLResourceStorageModeShared];
        memcpy([buffer->buffer contents], data, size);
        buffer->offset = 0;
    } else {
        buffer->heap = nil;
        buffer->buffer = [mtlDevice newBufferWithBytes:data length:size options:MTLResourceStorageModeShared];
        buffer->offset = 0;
    }

    return buffer;
}


RenderSetupInfoMTL::RenderSetupInfoMTL(id<MTLDevice> mtlDevice,id<MTLLibrary> mtlLibrary)
: mtlDevice(mtlDevice), heapManage(mtlDevice)
{
    memAlign = [mtlDevice heapBufferSizeAndAlignWithLength:1 options:MTLResourceUsageRead].align;
}
    
}
