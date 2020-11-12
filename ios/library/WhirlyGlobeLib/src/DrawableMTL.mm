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

ArgBuffContentsMTL::ArgBuffContentsMTL(id<MTLDevice> mtlDevice,RenderSetupInfoMTL *inSetupInfoMTL,id<MTLFunction> func,int bufferArgIdx,BufferBuilderMTL *buffBuild)
{
    valid = false;
    setupInfoMTL = inSetupInfoMTL;
    
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
        if (mem.dataType == MTLDataTypeStruct) {
            entries[mem.argumentIndex] = std::make_shared<Entry>(mem.argumentIndex);
        } else if (mem.dataType == MTLDataTypeBool) {
            constants.insert([mem.name cStringUsingEncoding:NSASCIIStringEncoding]);
        }
    }
    
    // Create a buffer to store the arguments in
    if (buffBuild)
        buffBuild->reserveData([encode encodedLength], &buff);
    
    isSetup = true;
    valid = true;
}

bool ArgBuffContentsMTL::isEmpty()
{
    return [encode encodedLength] == 0;
}

bool ArgBuffContentsMTL::hasEntry(int entryID)
{
    return entries.find(entryID) != entries.end();
}

bool ArgBuffContentsMTL::hasConstant(const std::string &name)
{
    return constants.find(name) != constants.end();
}

void ArgBuffContentsMTL::startEncoding(id<MTLDevice> mtlDevice)
{
    NSUInteger len = [encode encodedLength];
    if (len > 0) {
        tmpBuff = setupInfoMTL->heapManage.allocateBuffer(HeapManagerMTL::Drawable, len);
        [encode setArgumentBuffer:tmpBuff.buffer offset:tmpBuff.offset];
    }
}

void ArgBuffContentsMTL::endEncoding(id<MTLDevice> mtlDevice, id<MTLBlitCommandEncoder> blitEncode)
{
    if (!tmpBuff.buffer)
        return;
    [blitEncode copyFromBuffer:tmpBuff.buffer sourceOffset:tmpBuff.offset toBuffer:buff.buffer destinationOffset:buff.offset size:[tmpBuff.buffer length]];
}

void ArgBuffContentsMTL::updateEntry(id<MTLDevice> mtlDevice,id<MTLBlitCommandEncoder> blitEncode,int entryID,void *rawData,size_t size)
{
    const auto it = entries.find(entryID);
    if (it == entries.end())
        return;
    const auto entry = it->second;
    
    memcpy([encode constantDataAtIndex:entry->entryID], rawData, size);
}

void ArgBuffContentsMTL::addResources(ResourceRefsMTL &resources)
{
    resources.addEntry(buff);
    resources.addEntry(tmpBuff);
}

ArgBuffRegularTexturesMTL::ArgBuffRegularTexturesMTL(id<MTLDevice> mtlDevice, RenderSetupInfoMTL *setupInfoMTL, id<MTLFunction> mtlFunction, int bufferArgIdx, BufferBuilderMTL *buildBuff)
{
    encode = [mtlFunction newArgumentEncoderWithBufferIndex:bufferArgIdx];
    size = [encode encodedLength];
    buffer = setupInfoMTL->heapManage.allocateBuffer(HeapManagerMTL::HeapType::Drawable, size);
    if (buildBuff)
        buildBuff->reserveData(size, &buffer);
}

void ArgBuffRegularTexturesMTL::addTexture(const Point2f &offset,const Point2f &scale,id<MTLTexture> tex)
{
    offsets.push_back(offset);
    scales.push_back(scale);
    texs.push_back(tex);
}

void ArgBuffRegularTexturesMTL::updateBuffer(id<MTLDevice> mtlDevice,RenderSetupInfoMTL *setupInfoMTL,id<MTLBlitCommandEncoder> bltEncode)
{
    srcBuffer = setupInfoMTL->heapManage.allocateBuffer(HeapManagerMTL::Drawable, size);

    [encode setArgumentBuffer:srcBuffer.buffer offset:srcBuffer.offset];
    
    // TexIndirect constants first
    memcpy([encode constantDataAtIndex:WKSTexBuffIndirectOffset], &offsets[0], sizeof(float)*2*offsets.size());
    memcpy([encode constantDataAtIndex:WKSTexBuffIndirectScale], &scales[0], sizeof(float)*2*scales.size());

    // Then the textures, which are largely opaque
    uint32_t texturesPresent = 0;
    for (unsigned int ii=0;ii<WKSTextureMax;ii++) {
        id<MTLTexture> tex = ii>=texs.size() ? nil : texs[ii];
        [encode setTexture:tex atIndex:WKSTexBuffTextures+ii];
        if (tex) {
            // numTextures refers to the base textures, rather than program provide textures
            texturesPresent |= (1<<ii);
        }
    }
    memcpy([encode constantDataAtIndex:WKSTexBufTexPresent], &texturesPresent, sizeof(uint32_t));
    
    [bltEncode copyFromBuffer:srcBuffer.buffer sourceOffset:srcBuffer.offset toBuffer:buffer.buffer destinationOffset:buffer.offset size:size];
    
    offsets.clear();
    scales.clear();
    texs.clear();
}

size_t ArgBuffRegularTexturesMTL::encodedLength()
{
    return size;
}

void ArgBuffRegularTexturesMTL::addResources(ResourceRefsMTL &resources)
{
    resources.addEntry(srcBuffer);
    resources.addEntry(buffer);
}
    
}

