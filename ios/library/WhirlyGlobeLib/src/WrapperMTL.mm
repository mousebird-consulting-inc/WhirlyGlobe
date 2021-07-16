/*  WrapperMTL.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/16/19.
 *  Copyright 2011-2021 mousebird consulting
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

#import "WrapperMTL.h"
#import "TextureMTL.h"
#import "DrawableMTL.h"
#import "WhirlyKitLog.h"

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

void CopyIntoMtlFloat4x4Pair(simd::float4x4 &dest,simd::float4x4 &destDiff,const Eigen::Matrix4d &src)
{
    for (unsigned int ix=0;ix<4;ix++)
        for (unsigned int iy=0;iy<4;iy++) {
            const double val = src(ix*4+iy);
            const float fVal = val;
            const float fDiff = val - fVal;
            
            dest.columns[ix][iy] = fVal;
            destDiff.columns[ix][iy] = fDiff;
        }
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
: heap(nil), buffer(nil), offset(0), valid(false)
{
}

bool BufferEntryMTL::operator == (const BufferEntryMTL &that)
{
    return heap == that.heap && buffer == that.buffer && offset == that.offset;
}

void BufferEntryMTL::clear()
{
    heap = nil;  buffer = nil;  offset = 0;
    valid = false;
}

BufferBuilderMTL::BufferBuilderMTL(RenderSetupInfoMTL *setupInfo)
: data(nil), setupInfo(setupInfo)
{
    data = [NSMutableData new];
}

void BufferBuilderMTL::addData(const void *inData,size_t size,BufferEntryMTL *buffer)
{
    size_t start = [data length];
    [data appendBytes:inData length:size];

    // Keep the allocations on the alignment Metal wants
    size_t extra = size % setupInfo->memAlign;
    if (extra > 0) {
        [data increaseLengthBy:setupInfo->memAlign - extra];
        [data resetBytesInRange:NSMakeRange(start + size, setupInfo->memAlign - extra)];
    }

    buffer->offset = start;
    buffer->valid = true;
    bufferRefs.push_back(buffer);
}

void BufferBuilderMTL::reserveData(size_t size,BufferEntryMTL *buffer)
{
    // Keep the allocations on the alignment Metal wants
    size_t extra = size % setupInfo->memAlign;
    if (extra > 0) {
        size += setupInfo->memAlign - extra;
    }

    int start = [data length];
    [data increaseLengthBy:size];
    [data resetBytesInRange:NSMakeRange(start, size)];

    buffer->offset = start;
    buffer->valid = true;
    bufferRefs.push_back(buffer);
}

BufferEntryMTL BufferBuilderMTL::buildBuffer()
{
    if ([data length] == 0)
        return BufferEntryMTL();
        
    BufferEntryMTL buffer = setupInfo->heapManage.allocateBuffer(HeapManagerMTL::Drawable, [data mutableBytes],[data length]);
    
    // Update all the buffer references we already created
    for (auto bufRef : bufferRefs) {
        bufRef->heap = buffer.heap;
        bufRef->buffer = buffer.buffer;
    }

    bufferRefs.clear();
    
    return buffer;
}

TextureEntryMTL::TextureEntryMTL()
: heap(nil), tex(nil)
{
    
}

ResourceRefsMTL::ResourceRefsMTL(bool trackHolds)
: trackHolds(trackHolds)
{
    heaps = [[NSMutableSet alloc] init];
    buffers = [[NSMutableSet alloc] init];
    textures = [[NSMutableSet alloc] init];

    buffersToHold = [[NSMutableSet alloc] init];
    texturesToHold = [[NSMutableSet alloc] init];
}

void ResourceRefsMTL::addEntry(const BufferEntryMTL &entry)
{
    if (entry.heap)
        [heaps addObject:entry.heap];
    else if (entry.buffer)
        [buffers addObject:entry.buffer];
    
    if (trackHolds)
        [buffersToHold addObject:entry.buffer];
}

void ResourceRefsMTL::addBuffer(id<MTLBuffer> buffer)
{
    [buffers addObject:buffer];
}

void ResourceRefsMTL::addTexture(const TextureEntryMTL &tex)
{
    if (tex.heap)
        [heaps addObject:tex.heap];
    else
        [textures addObject:tex.tex];
    
    if (trackHolds)
        [texturesToHold addObject:tex.tex];
}

void ResourceRefsMTL::addTextures(const std::vector<TextureEntryMTL> &textures)
{
    for (const auto &tex: textures)
        addTexture(tex);
}

void ResourceRefsMTL::addResources(const ResourceRefsMTL &other)
{
    [heaps unionSet:other.heaps];
    [buffers unionSet:other.buffers];
    [textures unionSet:other.textures];
    [buffersToHold unionSet:other.buffersToHold];
    [texturesToHold unionSet:other.texturesToHold];
}

void ResourceRefsMTL::use(id<MTLRenderCommandEncoder> cmdEncode)
{
    if ([heaps count] == 0 && [buffers count] == 0 && [textures count] == 0)
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
    [heaps removeAllObjects];
    [buffers removeAllObjects];
    [textures removeAllObjects];
    [buffersToHold removeAllObjects];
    [texturesToHold removeAllObjects];
}

RenderTeardownInfoMTL::RenderTeardownInfoMTL()
{
    resources = std::make_shared<ResourceRefsMTL>(true);
}

void RenderTeardownInfoMTL::clear()
{
    resources->clear();

    // For Metal, we just drop the references and the rest is cleaned up
    for (auto &draw: drawables)
        draw->teardownForRenderer(nullptr, nullptr, nullptr);
    for (auto &tex: textures)
        tex->destroyInRenderer(nullptr, nullptr);
}

void RenderTeardownInfoMTL::destroyTexture(SceneRenderer *renderer,const TextureBaseRef &tex)
{
    textures.push_back(tex);
}

void RenderTeardownInfoMTL::destroyDrawable(SceneRenderer *renderer,const DrawableRef &draw)
{
    drawables.push_back(draw);
}


const bool HeapManagerMTL::UseHeaps =
#if TARGET_OS_SIMULATOR
    false;
#else
    true;
#endif

HeapManagerMTL::HeapManagerMTL(id<MTLDevice> mtlDevice)
: mtlDevice(mtlDevice)
{
    memAlign = [mtlDevice heapBufferSizeAndAlignWithLength:1 options:MTLResourceUsageRead].align;
}

void HeapManagerMTL::updateHeaps()
{
    for (unsigned int ig=0;ig<MaxType;ig++) {
        for (auto heap : heapGroups[ig].heaps) {
            heap->maxAvailSize = [heap->heap maxAvailableSizeWithAlignment:memAlign];
        }
    }
    
    for (auto texHeap : texGroups.heaps) {
        texHeap->maxAvailSize = [texHeap->heap maxAvailableSizeWithAlignment:memAlign];
    }
}

HeapManagerMTL::HeapInfoRef HeapManagerMTL::findHeap(HeapType heapType,size_t &size)
{
    HeapGroup &heapGroup = heapGroups[heapType];
    for (auto heap : heapGroup.heaps) {
        MTLSizeAndAlign sAlign = [mtlDevice heapBufferSizeAndAlignWithLength:size options:MTLResourceUsageRead];
        size = sAlign.size;
        if (heap->maxAvailSize > size) {
            heap->maxAvailSize -= size;
            return heap;
        }
    }
    MTLHeapDescriptor *heapDesc = [[MTLHeapDescriptor alloc] init];
    heapDesc.size = 32*1024*1024;  // 32MB, at a guess
    if (size > heapDesc.size) {
        heapDesc.size = size + 1024*1024;   // Silly to make a heap just for this, but this works
    }
    // TODO: Don't need this for most things
    heapDesc.storageMode = MTLStorageModeShared;
    HeapInfoRef heapInfo = std::make_shared<HeapInfo>();
    heapInfo->heap = [mtlDevice newHeapWithDescriptor:heapDesc];
    heapInfo->maxAvailSize = [heapInfo->heap maxAvailableSizeWithAlignment:memAlign];
    if (heapInfo->heap)
        heapGroup.heaps.insert(heapInfo);
    
    return heapInfo;
}

HeapManagerMTL::HeapInfoRef HeapManagerMTL::findTextureHeap(MTLTextureDescriptor *desc,size_t size)
{
    for (auto heap : texGroups.heaps) {
        MTLSizeAndAlign sAlign = [mtlDevice heapBufferSizeAndAlignWithLength:size options:MTLResourceUsageRead];
        size = sAlign.size;
        if (heap->maxAvailSize >= size) {
            heap->maxAvailSize -= size;
            return heap;
        }
    }
    
    MTLHeapDescriptor *heapDesc = [[MTLHeapDescriptor alloc] init];
    heapDesc.size = 32*1024*1024;  // 32MB, at a guess
    if (size > heapDesc.size) {
        heapDesc.size = size + 1024*1024;   // Silly to make a heap just for this, but this works
    }
    // TODO: Don't need this for most things
    heapDesc.storageMode = MTLStorageModeShared;
    HeapInfoRef heapInfo = std::make_shared<HeapInfo>();
    heapInfo->heap = [mtlDevice newHeapWithDescriptor:heapDesc];
    heapInfo->maxAvailSize = [heapInfo->heap maxAvailableSizeWithAlignment:memAlign];
    if (heapInfo->heap)
        texGroups.heaps.insert(heapInfo);

    return heapInfo;
}

BufferEntryMTL HeapManagerMTL::allocateBuffer(HeapType heapType,size_t size)
{
    BufferEntryMTL buffer;
    if (UseHeaps) {
        {
            std::lock_guard<std::mutex> guardLock(lock);
            bool keepTrying = true;
            while (keepTrying)
            {
                auto heapInfo = findHeap(heapType,size);
                buffer.heap = heapInfo->heap;
                buffer.buffer = [buffer.heap newBufferWithLength:size options:MTLResourceStorageModeShared];

                if (!buffer.buffer) {
    //                NSLog(@"Uh oh!  Ran out of buffer space [heap type %d, alloc %zu]", heapType, size);
                    heapInfo->maxAvailSize = 0;  // You lie!!!
                    keepTrying = true;
                    continue;
                }
                keepTrying = false;
            }
            buffer.offset = 0;
        }
    } else {
        size_t extra = size % memAlign;
        if (extra > 0) {
            size += memAlign - extra;
        }

        buffer.buffer = [mtlDevice newBufferWithLength:size options:MTLResourceStorageModeShared];
        buffer.heap = nil;
        buffer.offset = 0;
    }
    
    buffer.valid = true;
    return buffer;
}

BufferEntryMTL HeapManagerMTL::allocateBuffer(HeapType heapType,const void *data,size_t size)
{
    BufferEntryMTL buffer;
    if (UseHeaps) {
        {
            std::lock_guard<std::mutex> guardLock(lock);

            bool keepTrying = true;
            while (keepTrying) {
                auto heapInfo = findHeap(heapType,size);
                buffer.heap = heapInfo->heap;
                buffer.buffer = [buffer.heap newBufferWithLength:size options:MTLResourceStorageModeShared];

                if (!buffer.buffer) {
//                    NSLog(@"Uh oh!  Ran out of buffer space [heap type %d, alloc %zu]", heapType, size);
                    heapInfo->maxAvailSize = 0;  // Lies!  It's all lies!
                    keepTrying = true;
                    continue;
                }
                keepTrying = false;
            }
        }
        if (data && size) {
            memcpy([buffer.buffer contents], data, size);
        }
        buffer.offset = 0;
    } else {
        buffer.heap = nil;
        buffer.buffer = [mtlDevice newBufferWithBytes:data length:size options:MTLResourceStorageModeShared];
        buffer.offset = 0;
    }

    buffer.valid = true;
    return buffer;
}

TextureEntryMTL HeapManagerMTL::newTextureWithDescriptor(MTLTextureDescriptor *desc,size_t size)
{
    TextureEntryMTL tex;
    
    if (UseHeaps) {
        {
            std::lock_guard<std::mutex> guardLock(texLock);

            // It turns out that our estimates on size aren't valid for some formats, so try a few times with bigger estimates
            bool keepTrying = true;
            while (keepTrying) {
                auto heapInfo = findTextureHeap(desc, size);
                tex.heap = heapInfo->heap;
                tex.tex = [tex.heap newTextureWithDescriptor:desc];

                if (!tex.tex) {
                    heapInfo->maxAvailSize = 0;  // Lies!
                    keepTrying = true;
                    continue;;
                }
                keepTrying = false;
            }
        }
    } else {
        tex.tex = [mtlDevice newTextureWithDescriptor:desc];
    }

    if (!tex.tex)
    {
        wkLogLevel(Warn, "Failed to allocate texture (%lld)", size);
    }
    
    return tex;
}

RenderSetupInfoMTL::RenderSetupInfoMTL(id<MTLDevice> mtlDevice,
                                       id<MTLLibrary> mtlLibrary) :
    mtlDevice(mtlDevice),
    heapManage(mtlDevice)
{
    memAlign = [mtlDevice heapBufferSizeAndAlignWithLength:1 options:MTLResourceUsageRead].align;
}

static WhirlyKitShader::ExpType ToExpType(ExpressionInfoType type)
{
    switch (type)
    {
        case ExpressionNone:        return WhirlyKitShader::ExpNone;
        default:
        case ExpressionLinear:      return WhirlyKitShader::ExpLinear;
        case ExpressionExponential: return WhirlyKitShader::ExpExponential;
    }
}

void FloatExpressionToMtl(const FloatExpressionInfoRef &expInfo,WhirlyKitShader::FloatExp &mtlExp)
{
    mtlExp.type = ToExpType(expInfo->type);
    mtlExp.base = expInfo->base;
    mtlExp.numStops = std::min(std::min(expInfo->stopInputs.size(),expInfo->stopOutputs.size()),(size_t)WKSExpStops);
    for (unsigned int ii=0;ii<mtlExp.numStops;ii++)
    {
        mtlExp.stopInputs[ii] = expInfo->stopInputs[ii];
        mtlExp.stopOutputs[ii] = expInfo->stopOutputs[ii];
    }
}

void ColorExpressionToMtl(const ColorExpressionInfoRef &expInfo,WhirlyKitShader::ColorExp &mtlExp)
{
    mtlExp.type = ToExpType(expInfo->type);
    mtlExp.base = expInfo->base;
    mtlExp.numStops = std::min(std::min(expInfo->stopInputs.size(),expInfo->stopOutputs.size()),(size_t)WKSExpStops);
    for (unsigned int ii=0;ii<mtlExp.numStops;ii++)
    {
        mtlExp.stopInputs[ii] = expInfo->stopInputs[ii];
        float vals[4];
        expInfo->stopOutputs[ii].asUnitFloats(vals);
        for (unsigned int jj=0;jj<4;jj++)
        {
            mtlExp.stopOutputs[ii][jj] = vals[jj];
        }
    }
}

}
