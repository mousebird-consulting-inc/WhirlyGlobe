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

TextureEntryMTL::TextureEntryMTL()
: heap(nil), tex(nil)
{
    
}

void ResourceRefsMTL::addEntry(BufferEntryMTLRef entry)
{
    if (!entry)
        return;
    if (entry->heap)
        heaps.insert(entry->heap);
    else if (entry->buffer)
        buffers.insert(entry->buffer);
    
    buffersToHold.insert(entry->buffer);
}

void ResourceRefsMTL::addBuffer(id<MTLBuffer> buffer)
{
    buffers.insert(buffer);
}

void ResourceRefsMTL::addTexture(TextureEntryMTL &tex)
{
    if (tex.heap)
        heaps.insert(tex.heap);
    else
        textures.insert(tex.tex);
    
    texturesToHold.insert(tex.tex);
}

void ResourceRefsMTL::addTextures(const std::vector<TextureEntryMTL> &textures)
{
    for (auto tex: textures)
        addTexture(tex);
}

void ResourceRefsMTL::addResources(ResourceRefsMTL &other)
{
    heaps.insert(other.heaps.begin(),other.heaps.end());
    buffers.insert(other.buffers.begin(),other.buffers.end());
    textures.insert(other.textures.begin(),other.textures.end());
    buffersToHold.insert(other.buffersToHold.begin(),other.buffersToHold.end());
    texturesToHold.insert(other.texturesToHold.begin(),other.texturesToHold.end());
}

void ResourceRefsMTL::use(id<MTLRenderCommandEncoder> cmdEncode)
{
    if (heaps.empty() && buffers.empty() && textures.empty())
        return;
//    int count = 0;
//    id<MTLResource> all[heaps.size()+buffers.size()+textures.size()];
    
    for (id<MTLHeap> heap : heaps)
        [cmdEncode useHeap:heap];
    for (id<MTLBuffer> buff : buffers)
        [cmdEncode useResource:buff usage:MTLResourceUsageRead];
//        all[count++] = buff;
    for (id<MTLTexture> tex : textures)
        [cmdEncode useResource:tex usage:MTLResourceUsageRead];
//        all[count++] = tex;
    
//    [cmdEncode useResources:all count:count usage:MTLResourceUsageRead];
}

void ResourceRefsMTL::clear()
{
    heaps.clear();
    buffers.clear();
    textures.clear();
    buffersToHold.clear();
}

#if TARGET_OS_SIMULATOR
bool HeapManagerMTL::UseHeaps = false;
#else
bool HeapManagerMTL::UseHeaps = true;
#endif

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
    heapDesc.size = 32*1024*1024;  // 32MB, at a guess
    if (size > heapDesc.size) {
        heapDesc.size = size + 1024*1024;   // Silly to make a heap just for this, but this works
    }
    // TODO: Don't need this for most things
    heapDesc.storageMode = MTLStorageModeShared;
    id<MTLHeap> newHeap = [mtlDevice newHeapWithDescriptor:heapDesc];
    heapGroup.heaps.push_back(newHeap);
    
    return newHeap;
}

id<MTLHeap> HeapManagerMTL::findTextureHeap(MTLTextureDescriptor *desc,size_t size)
{
    for (auto heap : texGroups.heaps) {
        MTLSizeAndAlign sAlign = [mtlDevice heapBufferSizeAndAlignWithLength:size options:MTLResourceUsageRead];
        size = sAlign.size;
        if ([heap maxAvailableSizeWithAlignment:sAlign.align] >= size)
            return heap;
    }
    
    MTLHeapDescriptor *heapDesc = [[MTLHeapDescriptor alloc] init];
    heapDesc.size = 32*1024*1024;  // 32MB, at a guess
    if (size > heapDesc.size) {
        heapDesc.size = size + 1024*1024;   // Silly to make a heap just for this, but this works
    }
    // TODO: Don't need this for most things
    heapDesc.storageMode = MTLStorageModeShared;
    id<MTLHeap> newHeap = [mtlDevice newHeapWithDescriptor:heapDesc];
    texGroups.heaps.push_back(newHeap);
    
    return newHeap;
}

BufferEntryMTLRef HeapManagerMTL::allocateBuffer(HeapType heapType,size_t size)
{
    BufferEntryMTLRef buffer(new BufferEntryMTL());
    if (UseHeaps) {
        buffer->heap = findHeap(heapType,size);
        buffer->buffer = [buffer->heap newBufferWithLength:size options:MTLResourceStorageModeShared];
        if (!buffer->buffer) {
            NSLog(@"Uh oh!  Ran out of buffer space");
        }
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
        if (!buffer->buffer) {
            NSLog(@"Uh oh!  Ran out of buffer space");
        }
        memcpy([buffer->buffer contents], data, size);
        buffer->offset = 0;
    } else {
        buffer->heap = nil;
        buffer->buffer = [mtlDevice newBufferWithBytes:data length:size options:MTLResourceStorageModeShared];
        buffer->offset = 0;
    }

    return buffer;
}

TextureEntryMTL HeapManagerMTL::newTextureWithDescriptor(MTLTextureDescriptor *desc,size_t size)
{
    TextureEntryMTL tex;
    
    if (UseHeaps) {
        // It turns out that our estimates on size aren't valid for some formats, so try a few times with bigger estimates
        for (unsigned int ii=0;ii<3;ii++)
            if (!tex.tex) {
                tex.heap = findTextureHeap(desc, (1<<ii)*size);
                tex.tex = [tex.heap newTextureWithDescriptor:desc];
                if (tex.tex)
                    break;
            }
    } else {
        tex.tex = [mtlDevice newTextureWithDescriptor:desc];
    }
    
    return tex;
}

RenderSetupInfoMTL::RenderSetupInfoMTL(id<MTLDevice> mtlDevice,id<MTLLibrary> mtlLibrary)
: mtlDevice(mtlDevice), heapManage(mtlDevice)
{
    memAlign = [mtlDevice heapBufferSizeAndAlignWithLength:1 options:MTLResourceUsageRead].align;
}
    
void FloatExpressionToMtl(FloatExpressionInfoRef expInfo,WhirlyKitShader::FloatExp &mtlExp)
{
    switch (expInfo->type) {
        case ExpressionNone:
            mtlExp.type = WhirlyKitShader::ExpNone;
            break;
        case ExpressionLinear:
            mtlExp.type = WhirlyKitShader::ExpLinear;
            break;
        case ExpressionExponential:
            mtlExp.type = WhirlyKitShader::ExpExponential;
            break;
    }
    mtlExp.numStops = std::min(std::min(expInfo->stopInputs.size(),expInfo->stopOutputs.size()),(size_t)WKSExpStops);
    for (unsigned int ii=0;ii<mtlExp.numStops;ii++) {
        mtlExp.base = 1.0;
        mtlExp.stopInputs[ii] = expInfo->stopInputs[ii];
        mtlExp.stopOutputs[ii] = expInfo->stopOutputs[ii];
    }
}

void ColorExpressionToMtl(ColorExpressionInfoRef expInfo,WhirlyKitShader::ColorExp &mtlExp)
{
    switch (expInfo->type) {
        case ExpressionNone:
            mtlExp.type = WhirlyKitShader::ExpNone;
            break;
        case ExpressionLinear:
            mtlExp.type = WhirlyKitShader::ExpLinear;
            break;
        case ExpressionExponential:
            mtlExp.type = WhirlyKitShader::ExpExponential;
            break;
    }
    mtlExp.numStops = std::min(std::min(expInfo->stopInputs.size(),expInfo->stopOutputs.size()),(size_t)WKSExpStops);
    for (unsigned int ii=0;ii<mtlExp.numStops;ii++) {
        mtlExp.base = 1.0;
        mtlExp.stopInputs[ii] = expInfo->stopInputs[ii];
        float vals[4];
        expInfo->stopOutputs[ii].asUnitFloats(vals);
        for (unsigned int jj=0;jj<4;jj++)
            mtlExp.stopOutputs[ii][jj] = vals[jj];
    }
}

}
