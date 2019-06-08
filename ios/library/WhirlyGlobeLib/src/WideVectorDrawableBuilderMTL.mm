/*
 *  WideVectorDrawableBuilderMTL.mm
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

#import "WideVectorDrawableBuilderMTL.h"
#import "DefaultShadersMTL.h"
#import "ProgramMTL.h"

namespace WhirlyKit
{
    
// Metal version of the tweaker
void WideVectorTweakerMTL::tweakForFrame(Drawable *inDraw,RendererFrameInfo *inFrameInfo)
{
    if (!inFrameInfo->program || inFrameInfo->sceneRenderer->getType() != SceneRenderer::RenderMetal)
        return;
 
    RendererFrameInfoMTL *frameInfo = (RendererFrameInfoMTL *)inFrameInfo;
    
    float scale = std::max(frameInfo->sceneRenderer->framebufferWidth,frameInfo->sceneRenderer->framebufferHeight);
    float screenSize = frameInfo->screenSizeInDisplayCoords.x();
    float pixDispSize = std::min(frameInfo->screenSizeInDisplayCoords.x(),frameInfo->screenSizeInDisplayCoords.y()) / scale;
    float texScale = scale/(screenSize*texRepeat);
    
    WhirlyKitShader::UniformWideVec uniWV;
    if (realWidthSet)
    {
        uniWV.w2 = (float)(realWidth / pixDispSize);
        uniWV.real_w2 = realWidth;
    } else {
        uniWV.w2 = lineWidth;
        uniWV.real_w2 = pixDispSize * lineWidth;
    }
    uniWV.edge = edgeSize;
    uniWV.texScale = texScale;
    uniWV.color[0] = color.r/255.0;
    uniWV.color[1] = color.g/255.0;
    uniWV.color[2] = color.b/255.0;
    uniWV.color[3] = color.a/255.0;
    
    [frameInfo->cmdEncode setVertexBytes:&uniWV length:sizeof(uniWV) atIndex:WKSUniformDrawStateWideVecBuffer];
    [frameInfo->cmdEncode setFragmentBytes:&uniWV length:sizeof(uniWV) atIndex:WKSUniformDrawStateWideVecBuffer];
}

WideVectorDrawableBuilderMTL::WideVectorDrawableBuilderMTL(const std::string &name)
: BasicDrawableBuilderMTL(name)
{
}
    
void WideVectorDrawableBuilderMTL::Init(unsigned int numVert, unsigned int numTri, bool globeMode)
{
    basicDraw = new BasicDrawableMTL("Wide Vector");
    WideVectorDrawableBuilder::Init(numVert,numTri,globeMode);
    
    // Wire up the buffers
    // TODO: Merge these into a single data structure
    if (globeMode)
        ((VertexAttributeMTL *)basicDraw->vertexAttributes[basicDraw->normalEntry])->bufferIndex = WKSVertexNormalAttribute;
    ((VertexAttributeMTL *)basicDraw->vertexAttributes[basicDraw->colorEntry])->bufferIndex = WKSVertexColorAttribute;
    ((VertexAttributeMTL *)basicDraw->vertexAttributes[p1_index])->bufferIndex = WKSVertexWideVecP1Attribute;
    ((VertexAttributeMTL *)basicDraw->vertexAttributes[tex_index])->bufferIndex = WKSVertexWideVecTexInfoAttribute;
    ((VertexAttributeMTL *)basicDraw->vertexAttributes[n0_index])->bufferIndex = WKSVertexWideVecN0Attribute;
    ((VertexAttributeMTL *)basicDraw->vertexAttributes[c0_index])->bufferIndex = WKSVertexWideVecC0Attribute;
}

int WideVectorDrawableBuilderMTL::addAttribute(BDAttributeDataType dataType,StringIdentity nameID,int numThings)
{
    return BasicDrawableBuilderMTL::addAttribute(dataType, nameID, numThings);
}

WideVectorTweaker *WideVectorDrawableBuilderMTL::makeTweaker()
{
    return new WideVectorTweakerMTL();
}

BasicDrawable *WideVectorDrawableBuilderMTL::getDrawable()
{
    if (drawableGotten)
        return BasicDrawableBuilderMTL::getDrawable();
    
    BasicDrawable *theDraw = BasicDrawableBuilderMTL::getDrawable();
    setupTweaker(theDraw);
    
    return theDraw;
}

}
