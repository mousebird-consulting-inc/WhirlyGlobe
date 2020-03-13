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
        } else if (mem.dataType == MTLDataTypeTexture) {
            textures[mem.argumentIndex] = nil;
        }
    }
    
    // Create a buffer to store the arguments in
    buff = buffBuild.reserveData([encode encodedLength]);
    
    isSetup = false;
    valid = true;
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
    
    for (int texIndex=0;texIndex<textures.size();texIndex++)
        [encode setTexture:textures[texIndex] atIndex:WKSTextureArgBuffer+texIndex];
    
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

void ArgBuffContentsMTL::clearTextures()
{
    for (auto it : textures)
        it.second = nil;
}

void ArgBuffContentsMTL::setTexture(int texIndex,id<MTLTexture> tex)
{
    if (textures.find(texIndex) == textures.end())
        return;
    textures[texIndex] = tex;
    
    if (isSetup)
        [encode setTexture:tex atIndex:WKSTextureArgBuffer+texIndex];
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

