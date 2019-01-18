/*
 *  MaplyBaseInfo_iOS.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/18/19.
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

#import "MaplyBaseInfo_iOS_private.h"
#import "NSDictionary+Stuff.h"

namespace WhirlyKit {

void BaseInfoSetup(BaseInfo &baseInfo,NSDictionary *desc)
{
    baseInfo.minVis = [desc doubleForKey:@"minVis" default:DrawVisibleInvalid];
    baseInfo.maxVis = [desc doubleForKey:@"maxVis" default:DrawVisibleInvalid];
    baseInfo.minViewerDist = [desc doubleForKey:@"minviewerdist" default:DrawVisibleInvalid];
    baseInfo.maxViewerDist = [desc doubleForKey:@"maxviewerdist" default:DrawVisibleInvalid];
    baseInfo.viewerCenter.x() = [desc doubleForKey:@"viewablecenterx" default:DrawVisibleInvalid];
    baseInfo.viewerCenter.y() = [desc doubleForKey:@"viewablecentery" default:DrawVisibleInvalid];
    baseInfo.viewerCenter.z() = [desc doubleForKey:@"viewablecenterz" default:DrawVisibleInvalid];
    baseInfo.fade = [desc doubleForKey:@"fade" default:0.0];
    baseInfo.fadeIn = baseInfo.fade;
    baseInfo.fadeOut = baseInfo.fade;
    baseInfo.fadeIn = [desc doubleForKey:@"fadein" default:_fadeIn];
    baseInfo.fadeOut = [desc doubleForKey:@"fadeout" default:_fadeOut];
    baseInfo.fadeOutTime = [desc doubleForKey:@"fadeouttime" default:0.0];
    baseInfo.drawPriority = [desc intForKey:@"priority" default:0];
    baseInfo.drawPriority = [desc intForKey:@"drawPriority" default:_drawPriority];
    baseInfo.drawOffset = [desc doubleForKey:@"drawOffset" default:0.0];
    baseInfo.enable = [desc boolForKey:@"enable" default:true];
    baseInfo.startEnable = [desc doubleForKey:@"enablestart" default:0.0];
    baseInfo.endEnable = [desc doubleForKey:@"enableend" default:0.0];
    SimpleIdentity shaderID = [desc intForKey:@"shader" default:EmptyIdentity];
    baseInfo.programID = [desc intForKey:@"program" default:(int)shaderID];
    
    // Uniforms to be passed to shader
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
                valAttr.nameID = StringIndexer::getStringID([key cStringUsingEncoding:NSASCIIStringEncoding]);
                
                valAttr.type = BDFloatType;
                valAttr.data.floatVal = [val floatValue];
                
                baseInfo.uniforms.insert(valAttr);
            } else if ([val isKindOfClass:[UIColor class]])
            {
                SingleVertexAttribute valAttr;
                valAttr.nameID = StringIndexer::getStringID([key cStringUsingEncoding:NSASCIIStringEncoding]);
                
                UIColor *col = val;
                valAttr.type = BDChar4Type;
                RGBAColor color = [col asRGBAColor];
                valAttr.data.color[0] = color.r;
                valAttr.data.color[1] = color.g;
                valAttr.data.color[2] = color.b;
                valAttr.data.color[3] = color.a;
                
                baseInfo.uniforms.insert(valAttr);
            }
        }
    }
}
    
}
