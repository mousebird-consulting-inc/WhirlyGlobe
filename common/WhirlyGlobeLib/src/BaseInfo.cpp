/*
 *  BaseInfo.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/6/15.
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

#import "BaseInfo.h"
#import "Drawable.h"
#import "BasicDrawableBuilder.h"
#import "BasicDrawableInstanceBuilder.h"
#import "SharedAttributes.h"

using namespace WhirlyKit;
using namespace Eigen;

namespace WhirlyKit
{

BaseInfo::BaseInfo()
    : minVis(DrawVisibleInvalid), maxVis(DrawVisibleInvalid),
    minVisBand(DrawVisibleInvalid), maxVisBand(DrawVisibleInvalid),
    minViewerDist(DrawVisibleInvalid), maxViewerDist(DrawVisibleInvalid),
    viewerCenter(DrawVisibleInvalid,DrawVisibleInvalid,DrawVisibleInvalid),
    drawOffset(0.0),
    drawPriority(0),
    enable(true),
    fade(0.0), fadeIn(0.0), fadeOut(0.0), fadeOutTime(0.0),
    startEnable(0.0), endEnable(0.0),
    programID(EmptyIdentity),
    extraFrames(0),
    zBufferRead(false), zBufferWrite(false),
    renderTargetID(EmptyIdentity)
{
}
    
BaseInfo::BaseInfo(const Dictionary &dict)
{
    minVis = dict.getDouble(MaplyMinVis,DrawVisibleInvalid);
    maxVis = dict.getDouble(MaplyMaxVis,DrawVisibleInvalid);
    minVisBand = dict.getDouble(MaplyMinVisBand,DrawVisibleInvalid);
    maxVisBand = dict.getDouble(MaplyMaxVisBand,DrawVisibleInvalid);
    minViewerDist = dict.getDouble(MaplyMinViewerDist,DrawVisibleInvalid);
    maxViewerDist = dict.getDouble(MaplyMaxViewerDist,DrawVisibleInvalid);
    viewerCenter.x() = dict.getDouble(MaplyViewableCenterX,DrawVisibleInvalid);
    viewerCenter.y() = dict.getDouble(MaplyViewableCenterY,DrawVisibleInvalid);
    viewerCenter.z() = dict.getDouble(MaplyViewableCenterZ,DrawVisibleInvalid);
    fade = dict.getDouble(MaplyFade,0.0);
    fadeIn = fade;
    fadeOut = fade;
    fadeIn = dict.getDouble(MaplyFadeIn,fadeIn);
    fadeOut = dict.getDouble(MaplyFadeOut,fadeOut);
    fadeOutTime = dict.getDouble(MaplyFadeOutTime,0.0);
    drawPriority = dict.getInt("priority",0);
    drawPriority = dict.getInt(MaplyDrawPriority,drawPriority);
    drawOffset = dict.getDouble(MaplyDrawOffset,0.0);
    enable = dict.getBool(MaplyEnable,true);
    startEnable = dict.getDouble(MaplyEnableStart,0.0);
    endEnable = dict.getDouble(MaplyEnableEnd,0.0);
    SimpleIdentity shaderID = dict.getInt(MaplyShaderString,EmptyIdentity);
    programID = dict.getInt("program",shaderID);
    extraFrames = dict.getInt("extraFrames",0);
    zBufferRead = dict.getBool(MaplyZBufferRead,false);
    zBufferWrite = dict.getBool(MaplyZBufferWrite, false);
    renderTargetID = dict.getInt(MaplyRenderTargetDesc,EmptyIdentity);

    // Note: Porting
    // Uniforms to be passed to shader
#if 0
    NSDictionary *uniformDict = desc[@"shaderuniforms"];
    if (uniformDict)
    {
        for (NSString *key in uniformDict.allKeys)
        {
            id val = uniformDict[key];
            if ([val isKindOfClass:[NSNumber class]])
            {
                SingleVertexAttribute valAttr;
                valAttr.name = [key cStringUsingEncoding:NSASCIIStringEncoding];
                
                NSNumber *num = val;
                valAttr.type = BDFloatType;
                valAttr.data.floatVal = [val floatValue];
                
                _uniforms.insert(valAttr);
            } else if ([val isKindOfClass:[UIColor class]])
            {
                SingleVertexAttribute valAttr;
                valAttr.name = [key cStringUsingEncoding:NSASCIIStringEncoding];
                
                UIColor *col = val;
                valAttr.type = BDChar4Type;
                RGBAColor color = [col asRGBAColor];
                valAttr.data.color[0] = color.r;
                valAttr.data.color[1] = color.g;
                valAttr.data.color[2] = color.b;
                valAttr.data.color[3] = color.a;
                
                _uniforms.insert(valAttr);
            }
        }
    }
#endif
}
    
// Really Android?  Really?
template <typename T>
std::string to_string(T value)
{
    std::ostringstream os ;
    os << value ;
    return os.str() ;
}

std::string BaseInfo::toString()
{
    std::string outStr =
    (std::string)"minVis = " + to_string(minVis) + ";" +
    " maxVis = " + to_string(maxVis) + ";" +
    " minVisBand = " + to_string(minVisBand) + ";" +
    " maxVisBand = " + to_string(maxVisBand) + ";" +
    " minViewerDist = " + to_string(minViewerDist) + ";" +
    " maxViewerDist = " + to_string(maxViewerDist) + ";" +
    " viewerCenter = (" + to_string(viewerCenter.x()) + "," + to_string(viewerCenter.y()) + "," + to_string(viewerCenter.z()) + ");" +
    " drawOffset = " + to_string(drawOffset) + ";" +
    " drawPriority = " + to_string(drawPriority) + ";" +
    " enable = " + (enable ? "yes" : "no") + ";" +
    " fade = " + to_string(fade) + ";" +
    " fadeIn = " + to_string(fadeIn) + ";" +
    " fadeOut = " + to_string(fadeOut) + ";" +
    " fadeOutTime = " + to_string(fadeOutTime) + ";" +
    " startEnable = " + to_string(startEnable) + ";" +
    " endEnable = " + to_string(endEnable) + ";" +
    " programID = " + to_string(programID) + ";";
    
    return outStr;
}
    
void BaseInfo::setupBasicDrawable(BasicDrawableBuilderRef drawBuild) const
{
    setupBasicDrawable(drawBuild.get());
}

void BaseInfo::setupBasicDrawable(BasicDrawableBuilder *drawBuild) const
{
    drawBuild->setOnOff(enable);
    if (startEnable != endEnable)
        drawBuild->setEnableTimeRange(startEnable, endEnable);
    drawBuild->setDrawPriority(drawPriority);
    drawBuild->setVisibleRange(minVis,maxVis);
    drawBuild->setViewerVisibility(minViewerDist,maxViewerDist,viewerCenter);
    drawBuild->setProgram(programID);
    drawBuild->setUniforms(uniforms);
    drawBuild->setRequestZBuffer(zBufferRead);
    drawBuild->setWriteZBuffer(zBufferWrite);
    drawBuild->setProgram(programID);
    drawBuild->setExtraFrames(extraFrames);
    if (renderTargetID != EmptyIdentity)
        drawBuild->setRenderTarget(renderTargetID);
}

void BaseInfo::setupBasicDrawableInstance(BasicDrawableInstanceBuilderRef drawBuild) const
{
    setupBasicDrawableInstance(drawBuild.get());
}
    
void BaseInfo::setupBasicDrawableInstance(BasicDrawableInstanceBuilder *drawBuild) const
{
    drawBuild->setOnOff(enable);
    drawBuild->setEnableTimeRange(startEnable, endEnable);
    drawBuild->setDrawPriority(drawPriority);
    drawBuild->setVisibleRange(minVis,maxVis);
    drawBuild->setViewerVisibility(minViewerDist,maxViewerDist,viewerCenter);
    drawBuild->setUniforms(uniforms);
    drawBuild->setRequestZBuffer(zBufferRead);
    drawBuild->setWriteZBuffer(zBufferWrite);
    drawBuild->setProgram(programID);
    if (renderTargetID != EmptyIdentity)
        drawBuild->setRenderTarget(renderTargetID);
}
    
}
