/*  BaseInfo.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/6/15.
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

#import "BaseInfo.h"
#import "Drawable.h"
#import "BasicDrawableBuilder.h"
#import "BasicDrawableInstanceBuilder.h"
#import "SharedAttributes.h"

using namespace WhirlyKit;
using namespace Eigen;

namespace WhirlyKit
{

ExpressionInfo::ExpressionInfo() :
    type(ExpressionNone),
    base(1.0)
{
}

ExpressionInfo::ExpressionInfo(const ExpressionInfo &that) : // NOLINT(bugprone-copy-constructor-init)
    Identifiable(),  // Generate a new ID, don't copy
    type(that.type),
    base(that.base),
    stopInputs(that.stopInputs)
{
}

void FloatExpressionInfo::scaleBy(double scale)
{
    for (float &stopOutput : stopOutputs)
    {
        stopOutput = (float)(stopOutput * scale);
    }
}

template <typename TFrom,typename TTo>
static TTo evalExpr(float zoom, float base,TFrom defaultValue,
                    const std::vector<float> &inputs,const std::vector<TFrom> &outputs,
                    std::function<TTo(TFrom)> convert,
                    std::function<TTo(TFrom,TFrom,double)> interp)
{
    if (inputs.empty() || outputs.empty())
    {
        return convert(defaultValue);
    }
    if (zoom <= inputs.front())
    {
        return convert(outputs.front());
    }
    if (zoom > inputs.back())
    {
        return convert(outputs.back());
    }

    const auto numStops = std::min(inputs.size(), outputs.size());
    for (int i = 0; i < numStops - 1; ++i)
    {
        if (inputs[i] <= zoom && zoom < inputs[i + 1])
        {
            const auto zoomA = inputs[i];
            const auto zoomB = inputs[i + 1];
            const auto valA = outputs[i];
            const auto valB = outputs[i + 1];
            const auto t = (zoom - zoomA) / (zoomB - zoomA);
            if (base > 0)
            {
                // exponential
                if (base == 1.0f)
                {
                    return interp(valA, valB, t);
                }
                const auto a = std::pow(base, zoom - zoomA) - 1.0;
                const auto b = std::pow(base, zoomB - zoomA) - 1.0;
                return interp(valA, valB, a/b);
            }
            else
            {
                // linear
                return interp(valA, valB, t);
            }
        }
    }
    return convert(outputs.back());
}

// std::lerp is C++20
template <typename T> T static lerp(T a, T b, double t) { return (T)(t * (b - a) + a); }
static const auto flerp = lerp<float>;

template <typename T> T static inline ident(T t) { return t; }

static inline Vector4f toVec(RGBAColor c) { return c.asRGBAVecF(); }

static inline RGBAColor lerpRGB(RGBAColor a, RGBAColor b, double t)
{
    return RGBAColor{lerp(a.r,b.r,t),lerp(a.g,b.g,t),lerp(a.b,b.b,t),lerp(a.a,b.a,t)};
}

static inline Vector4f lerpVec(RGBAColor a, RGBAColor b, double t)
{
    return Vector4f{flerp(a.r,b.r,t)/255.f,flerp(a.g,b.g,t)/255.f,
                    flerp(a.b,b.b,t)/255.f,flerp(a.a,b.a,t)/255.f};
}

float FloatExpressionInfo::evaluate(float zoom, float defaultValue)
{
    return evalExpr<float,float>(zoom,base,defaultValue,stopInputs,stopOutputs,ident<float>,flerp);
}

RGBAColor ColorExpressionInfo::evaluate(float zoom, RGBAColor def)
{
    return evalExpr<RGBAColor,RGBAColor>(zoom,base,def,stopInputs,stopOutputs,ident<RGBAColor>,lerpRGB);
}

Vector4f ColorExpressionInfo::evaluateF(float zoom, RGBAColor def)
{
    return evalExpr<RGBAColor,Vector4f>(zoom,base,def,stopInputs,stopOutputs,toVec,lerpVec);
}

BaseInfo::BaseInfo()
    : minVis(DrawVisibleInvalid), maxVis(DrawVisibleInvalid),
    minVisBand(DrawVisibleInvalid), maxVisBand(DrawVisibleInvalid),
    minViewerDist(DrawVisibleInvalid), maxViewerDist(DrawVisibleInvalid),
    zoomSlot(-1),minZoomVis(DrawVisibleInvalid),maxZoomVis(DrawVisibleInvalid),
    viewerCenter(DrawVisibleInvalid,DrawVisibleInvalid,DrawVisibleInvalid),
    drawOffset(0.0),
    drawPriority(0),
    enable(true),
    fade(0.0), fadeIn(0.0), fadeOut(0.0), fadeOutTime(0.0),
    startEnable(0.0), endEnable(0.0),
    programID(EmptyIdentity),
    extraFrames(0),
    zBufferRead(false), zBufferWrite(false),
    renderTargetID(EmptyIdentity),
    hasExp(false)
{
}

BaseInfo::BaseInfo(const BaseInfo &that)
: minVis(that.minVis), maxVis(that.minVis), minVisBand(that.minVisBand), maxVisBand(that.maxVisBand),
  minViewerDist(that.minViewerDist), maxViewerDist(that.maxViewerDist), zoomSlot(that.zoomSlot),
  minZoomVis(that.minZoomVis),maxZoomVis(that.maxZoomVis), viewerCenter(that.viewerCenter),
  drawOffset(that.drawOffset), drawPriority(that.drawPriority), drawOrder(that.drawOrder),
  enable(that.enable), fade(that.fade), fadeIn(that.fadeIn), fadeOut(that.fadeOut),
  fadeOutTime(that.fadeOutTime), startEnable(that.startEnable), endEnable(that.endEnable),
  programID(that.programID), extraFrames(that.extraFrames), zBufferRead(that.zBufferRead),
  zBufferWrite(that.zBufferWrite), renderTargetID(that.renderTargetID), hasExp(that.hasExp)
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
    zoomSlot = dict.getInt(MaplyZoomSlot,-1);
    minZoomVis = dict.getDouble(MaplyMinZoomVis,DrawVisibleInvalid);
    maxZoomVis = dict.getDouble(MaplyMaxZoomVis,DrawVisibleInvalid);
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
    drawOrder = dict.getInt64(MaplyDrawOrder, DrawOrderTiles);
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
    hasExp = false;

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
    std::ostringstream os;
    os << value;
    return os.str();
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
    " zoomSlot = " + to_string(zoomSlot) + ";" +
    " minZoomVis = " + to_string(minZoomVis) + ";" +
    " maxZoomVis = " + to_string(maxZoomVis) + ";" +
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
    
void BaseInfo::setupBasicDrawable(const BasicDrawableBuilderRef &drawBuild) const
{
    setupBasicDrawable(drawBuild.get());
}

void BaseInfo::setupBasicDrawable(BasicDrawableBuilder *drawBuild) const
{
    drawBuild->setOnOff(enable);
    if (startEnable != endEnable)
        drawBuild->setEnableTimeRange(startEnable, endEnable);
    drawBuild->setDrawOrder(drawOrder);
    drawBuild->setDrawPriority(drawPriority);
    drawBuild->setVisibleRange((float)minVis,(float)maxVis);
    drawBuild->setViewerVisibility(minViewerDist,maxViewerDist,viewerCenter);
    drawBuild->setZoomInfo(zoomSlot,minZoomVis,maxZoomVis);
    drawBuild->setProgram(programID);
    drawBuild->setUniforms(uniforms);
    drawBuild->setRequestZBuffer(zBufferRead);
    drawBuild->setWriteZBuffer(zBufferWrite);
    drawBuild->setExtraFrames(extraFrames);
    if (renderTargetID != EmptyIdentity)
        drawBuild->setRenderTarget(renderTargetID);
    if (hasExp)
        drawBuild->setIncludeExp(true);
}

void BaseInfo::setupBasicDrawableInstance(const BasicDrawableInstanceBuilderRef &drawBuild) const
{
    setupBasicDrawableInstance(drawBuild.get());
}
    
void BaseInfo::setupBasicDrawableInstance(BasicDrawableInstanceBuilder *drawBuild) const
{
    drawBuild->setOnOff(enable);
    drawBuild->setEnableTimeRange(startEnable, endEnable);
    drawBuild->setDrawOrder(drawOrder);
    drawBuild->setDrawPriority(drawPriority);
    drawBuild->setVisibleRange((float)minVis,(float)maxVis);
    drawBuild->setViewerVisibility(minViewerDist,maxViewerDist,viewerCenter);
    drawBuild->setZoomInfo(zoomSlot,minZoomVis,maxZoomVis);
    drawBuild->setUniforms(uniforms);
    drawBuild->setRequestZBuffer(zBufferRead);
    drawBuild->setWriteZBuffer(zBufferWrite);
    drawBuild->setProgram(programID);
    if (renderTargetID != EmptyIdentity)
        drawBuild->setRenderTarget(renderTargetID);
//    if (hasExp)
//        drawBuild->setIncludeExp(true);
}
    
}
