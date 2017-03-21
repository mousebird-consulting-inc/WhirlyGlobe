/*
 *  BaseInfo.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/6/15.
 *  Copyright 2011-2016 mousebird consulting
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
#import "BasicDrawable.h"
#import "BasicDrawableInstance.h"

using namespace WhirlyKit;
using namespace Eigen;

namespace WhirlyKit
{
BaseInfo::BaseInfo()
    : minVis(DrawVisibleInvalid), maxVis(DrawVisibleInvalid), minVisBand(DrawVisibleInvalid), maxVisBand(DrawVisibleInvalid),
    minViewerDist(DrawVisibleInvalid), maxViewerDist(DrawVisibleInvalid), viewerCenter(DrawVisibleInvalid,DrawVisibleInvalid,DrawVisibleInvalid), fade(0.0), fadeIn(0.0), fadeOut(0.0), fadeOutTime(0.0), drawPriority(0), drawOffset(0.0),
    enable(true), startEnable(0.0), endEnable(0.0), programID(EmptyIdentity)
{
}

BaseInfo::BaseInfo(const Dictionary &dict)
{
    minVis = dict.getDouble("minVis",DrawVisibleInvalid);
    maxVis = dict.getDouble("maxVis",DrawVisibleInvalid);
    minVisBand = dict.getDouble("minVisBand",DrawVisibleInvalid);
    maxVisBand = dict.getDouble("maxVisBand",DrawVisibleInvalid);
    minViewerDist = dict.getDouble("minviewerdist",DrawVisibleInvalid);
    maxViewerDist = dict.getDouble("maxviewerdist",DrawVisibleInvalid);
    viewerCenter.x() = dict.getDouble("viewablecenterx",DrawVisibleInvalid);
    viewerCenter.y() = dict.getDouble("viewablecentery",DrawVisibleInvalid);
    viewerCenter.z() = dict.getDouble("viewablecenterz",DrawVisibleInvalid);
    fade = dict.getDouble("fade",0.0);
    fadeIn = fade;
    fadeOut = fade;
    fadeIn = dict.getDouble("fadein",fadeIn);
    fadeOut = dict.getDouble("fadeout",fadeOut);
    fadeOutTime = dict.getDouble("fadeouttime",0.0);
    drawPriority = dict.getInt("priority",0);
    drawPriority = dict.getInt("drawPriority",drawPriority);
    drawOffset = dict.getDouble("drawOffset",0.0);
    enable = dict.getBool("enable",true);
    startEnable = dict.getDouble("enablestart",0.0);
    endEnable = dict.getDouble("enableend",0.0);
    SimpleIdentity shaderID = dict.getInt("shader",EmptyIdentity);
    programID = dict.getInt("program",shaderID);

    // Note: Porting
    // Uniforms to be passed to shader
#if 0
    // Note: Should add the rest of the types
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

void BaseInfo::setupBasicDrawable(BasicDrawable *drawable) const
{
    drawable->setOnOff(enable);
    drawable->setEnableTimeRange(startEnable, endEnable);
    drawable->setDrawPriority(drawPriority);
    drawable->setVisibleRange(minVis,maxVis);
    drawable->setViewerVisibility(minViewerDist,maxViewerDist,viewerCenter);
    drawable->setProgram(programID);
    drawable->setUniforms(uniforms);
}

void BaseInfo::setupBasicDrawableInstance(BasicDrawableInstance *drawInst)
{
    drawInst->setEnable(enable);
    drawInst->setEnableTimeRange(startEnable, endEnable);
    drawInst->setDrawPriority(drawPriority);
    drawInst->setVisibleRange(minVis,maxVis);
    drawInst->setViewerVisibility(minViewerDist,maxViewerDist,viewerCenter);
    drawInst->setUniforms(uniforms);
}
    
}
