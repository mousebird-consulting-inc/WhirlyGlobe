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
}

BufferEntryMTLRef BufferBuilderMTL::reserveData(size_t size)
{
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
}

BufferEntryMTLRef BufferBuilderMTL::buildBuffer()
{
    if ([data length] == 0)
        return BufferEntryMTLRef();
        
    BufferEntryMTLRef buffer = setupInfo->heapManage.allocateBuffer(HeapManagerMTL::Drawable, [data mutableBytes],[data length]);
    
    // Update all the buffer references we already created
    for (auto bufRef : bufferRefs) {
        bufRef->heap = buffer->heap;
        bufRef->buffer = buffer->buffer;
    }
    bufferRefs.clear();

    return buffer;
}


void ResourceRefsMTL::addEntry(BufferEntryMTLRef entry)
{
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

void ResourceRefsMTL::use(id<MTLRenderCommandEncoder> cmdEncode)
{
    for (id<MTLHeap> heap : heaps)
        [cmdEncode useHeap:heap];
    for (id<MTLBuffer> buff : buffers)
        [cmdEncode useResource:buff usage:MTLResourceUsageRead];
    for (id<MTLTexture> tex : textures)
        [cmdEncode useResource:tex usage:MTLResourceUsageRead];
}

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
    buffer->heap = findHeap(heapType,size);
    buffer->buffer = [buffer->heap newBufferWithLength:size options:MTLResourceStorageModeShared];
    buffer->offset = 0;
    
    return buffer;
}

BufferEntryMTLRef HeapManagerMTL::allocateBuffer(HeapType heapType,void *data,size_t size)
{
    BufferEntryMTLRef buffer(new BufferEntryMTL());
    buffer->heap = findHeap(heapType,size);
    buffer->buffer = [buffer->heap newBufferWithLength:size options:MTLResourceStorageModeShared];
    memcpy([buffer->buffer contents], data, size);
    buffer->offset = 0;

    return buffer;
}


RenderSetupInfoMTL::RenderSetupInfoMTL(id<MTLDevice> mtlDevice)
: mtlDevice(mtlDevice), heapManage(mtlDevice)
{
    memAlign = [mtlDevice heapBufferSizeAndAlignWithLength:1 options:MTLResourceUsageRead].align;
}

RenderSetupInfoMTL::RenderSetupInfoMTL(Scene *scene,id<MTLDevice> mtlDevice)
: mtlDevice(mtlDevice), heapManage(mtlDevice)
{
}
    
}
