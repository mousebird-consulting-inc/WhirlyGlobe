/*
 *  DrawableMTL.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/10/20.
 *  Copyright 2011-2020 mousebird consulting
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

#import "DrawableMTL.h"

using namespace Eigen;

namespace WhirlyKit
{

ArgBuffContentsMTL::ArgBuffContentsMTL(id<MTLDevice> mtlDevice,RenderSetupInfoMTL *setupInfoMTL,id<MTLFunction> func,int bufferArgIdx,BufferBuilderMTL &buffBuild)
{
    valid = false;
    
    MTLAutoreleasedArgument argInfo;
    encode = [func newArgumentEncoderWithBufferIndex:bufferArgIdx reflection:&argInfo];
    if (!encode)
        return;
    
    // Figure out which entries are allowed within the argument buffer
    if (argInfo.bufferDataType != MTLDataTypeStruct) {
        NSLog(@"Unexpected buffer data type in Metal Function %@",func.name);
    }
    NSArray<MTLStructMember *> *members = argInfo.bufferStructType.members;
    if (!members) {
        NSLog(@"Unexpected buffer structure in Metal Function %@",func.name);
    }
    
    // Work through the members, adding an entry for each one we're to fill in
    for (MTLStructMember *mem in members) {
        if (mem.dataType == MTLDataTypePointer) {
            EntryRef entry(new Entry());
            entry->entryID = mem.argumentIndex;
            entry->size = mem.pointerType.dataSize;
            entries[entry->entryID] = entry;
        }
    }
    
    // Create a buffer to store the arguments in
    buff = buffBuild.reserveData([encode encodedLength]);
    
    isSetup = false;
    valid = true;
}

id<MTLArgumentEncoder> ArgBuffContentsMTL::getEncoderFor(SceneRendererMTL *sceneRender,int entryID)
{
    id<MTLArgumentEncoder> newEncode = [encode newArgumentEncoderForBufferAtIndex:entryID];
    return newEncode;
}

void ArgBuffContentsMTL::createBuffers(id<MTLDevice> mtlDevice,BufferBuilderMTL &buffBuild)
{
    // Create buffers for entries that don't have them
    for (auto it : entries) {
        auto entry = it.second;
        if (!entry->buffer) {
            entry->buffer = buffBuild.reserveData(entry->size);
        }
    }
}

void ArgBuffContentsMTL::setEntry(int entryID,BufferEntryMTLRef buffer)
{
    auto it = entries.find(entryID);
    if (it == entries.end())
        return;
    auto entry = it->second;
    entry->buffer = buffer;
}

void ArgBuffContentsMTL::wireUpBuffers()
{
    [encode setArgumentBuffer:buff->buffer offset:buff->offset];

    for (auto it : entries) {
        auto entry = it.second;
        [encode setBuffer:entry->buffer->buffer offset:entry->buffer->offset atIndex:entry->entryID];
    }
            
    isSetup = true;
}

bool ArgBuffContentsMTL::hasEntry(int entryID)
{
    return entries.find(entryID) != entries.end();
}

void ArgBuffContentsMTL::updateEntry(id<MTLDevice> mtlDevice,id<MTLBlitCommandEncoder> blitEncode,int entryID,void *rawData,size_t size)
{
    auto it = entries.find(entryID);
    if (it == entries.end())
        return;
    auto entry = it->second;
    
    // TODO: Try to reuse these buffers
    // This schedules a copy from our buffer to the other, thus not interfering with an ongoing render
    id<MTLBuffer> srcBuffer = [mtlDevice newBufferWithBytes:rawData length:size options:MTLResourceStorageModeShared];
    [blitEncode copyFromBuffer:srcBuffer sourceOffset:0 toBuffer:entry->buffer->buffer destinationOffset:entry->buffer->offset size:size];
}

void ArgBuffContentsMTL::updateEntry(id<MTLDevice> mtlDevice,id<MTLBlitCommandEncoder> blitEncode,int entryID,BufferEntryMTLRef buffer,size_t size)
{
    auto it = entries.find(entryID);
    if (it == entries.end())
        return;
    auto entry = it->second;

    [blitEncode copyFromBuffer:buffer->buffer sourceOffset:buffer->offset toBuffer:entry->buffer->buffer destinationOffset:entry->buffer->offset size:size];
}

void ArgBuffContentsMTL::addResources(ResourceRefsMTL &resources)
{
    for (auto it : entries) {
        resources.addEntry(it.second->buffer);
    }
}

BufferEntryMTLRef ArgBuffContentsMTL::getBuffer()
{
    return buff;
}

ArgBuffRegularTexturesMTL::ArgBuffRegularTexturesMTL(id<MTLDevice> mtlDevice,RenderSetupInfoMTL *setupInfoMTL,BufferBuilderMTL &buildBuff)
{
    NSMutableArray<MTLArgumentDescriptor *> *args = [NSMutableArray array];

    // Textures themselves
    for (unsigned int ii=0;ii<WKSTextureMax;ii++) {
        MTLArgumentDescriptor *tex = [[MTLArgumentDescriptor alloc] init];
        tex.access = MTLArgumentAccessReadOnly;
        tex.dataType = MTLDataTypeTexture;
        tex.index = WKSTexBuffTextures+ii;
        tex.textureType = MTLTextureType2D;
        [args addObject:tex];
    }
    // TexIndirect objects
    for (unsigned int ii=0;ii<WKSTextureMax;ii++) {
        MTLArgumentDescriptor *float2a = [[MTLArgumentDescriptor alloc] init];
        float2a.access = MTLArgumentAccessReadOnly;
        float2a.dataType = MTLDataTypeFloat2;
        float2a.index = WKSTexBuffIndirectOffset+ii;
        [args addObject:float2a];
    }
    for (unsigned int ii=0;ii<WKSTextureMax;ii++) {
        MTLArgumentDescriptor *float2a = [[MTLArgumentDescriptor alloc] init];
        float2a.access = MTLArgumentAccessReadOnly;
        float2a.dataType = MTLDataTypeFloat2;
        float2a.index = WKSTexBuffIndirectScale+ii;
        [args addObject:float2a];
    }
    // Number of textures at start
    MTLArgumentDescriptor *numTexturesArg = [[MTLArgumentDescriptor alloc] init];
    numTexturesArg.access = MTLArgumentAccessReadOnly;
    numTexturesArg.dataType = MTLDataTypeInt;
    numTexturesArg.index = WKSTexBufNumTextures;
    [args addObject:numTexturesArg];
    
    encode = [mtlDevice newArgumentEncoderWithArguments:args];
    size = [encode encodedLength];
    buffer = buildBuff.reserveData(size);
}

void ArgBuffRegularTexturesMTL::wireUpBuffer()
{
    [encode setArgumentBuffer:buffer->buffer offset:buffer->offset];
}

void ArgBuffRegularTexturesMTL::addTexture(const Point2f &offset,const Point2f &scale,id<MTLTexture> tex)
{
    offsets.push_back(offset);
    scales.push_back(scale);
    texs.push_back(tex);
}

BufferEntryMTLRef ArgBuffRegularTexturesMTL::encodeBuffer(RenderSetupInfoMTL *setupInfoMTL,id<MTLDevice> mtlDevice)
{
    // TexIndirect constants first
    memcpy([encode constantDataAtIndex:WKSTexBuffIndirectOffset], &offsets[0], sizeof(simd::float2)*offsets.size());
    memcpy([encode constantDataAtIndex:WKSTexBuffIndirectScale], &scales[0], sizeof(simd::float2)*scales.size());

    // Then the textures, which are largely opaque
    int numTextures = 0;
    for (unsigned int ii=0;ii<WKSTextureMax;ii++) {
        id<MTLTexture> tex = ii>=texs.size() ? nil : texs[ii];
        [encode setTexture:tex atIndex:WKSTexBuffTextures+ii];
        if (tex)
            numTextures++;
    }
    memcpy([encode constantDataAtIndex:WKSTexBufNumTextures], &numTextures, sizeof(numTextures));
    
    return buffer;
}

size_t ArgBuffRegularTexturesMTL::encodedLength()
{
    return size;
}

void DrawableMTL::encodeDirectCalculate(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode,Scene *scene)
{
}

void DrawableMTL::encodeDirect(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode,Scene *scene)
{
}

void DrawableMTL::encodeInirectCalculate(id<MTLIndirectRenderCommand> cmdEncode,SceneRendererMTL *sceneRender,Scene *scene,RenderTargetMTL *renderTarget)
{
}

void DrawableMTL::encodeIndirect(id<MTLIndirectRenderCommand> cmdEncode,SceneRendererMTL *sceneRender,Scene *scene,RenderTargetMTL *renderTarget)
{
}

    
}

