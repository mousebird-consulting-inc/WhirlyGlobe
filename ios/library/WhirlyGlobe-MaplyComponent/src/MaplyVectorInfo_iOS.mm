/*
 *  MaplyVectorInfo_iOS_private.h
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
#import "MaplyVectorInfo_iOS_private.h"
#import "UIColor+Stuff.h"
#import "NSDictionary+Stuff.h"

namespace WhirlyKit {
    
VectorInfo_iOS::VectorInfo_iOS(NSDictionary *desc)
{
    BaseInfoSetup(*this,desc);
    
    UIColor *theColor = [desc objectForKey:@"color" checkType:[UIColor class] default:[UIColor whiteColor]];
    if (theColor)
        color = [theColor asRGBAColor];
    drawOffset = [desc floatForKey:@"drawOffset" default:0];
    lineWidth = [desc floatForKey:@"width" default:1.0];
    filled = [desc boolForKey:@"filled" default:false];
    sample = [desc floatForKey:@"sample" default:false];
    texId = [desc intForKey:@"texture" default:EmptyIdentity];
    texScale.x() = [desc floatForKey:@"texscalex" default:1.0];
    texScale.y() = [desc floatForKey:@"texscaley" default:1.0];
    subdivEps = [desc floatForKey:@"subdivisionepsilon" default:0.0];
    NSString *subdivType = [desc stringForKey:@"subdivisiontype" default:nil];
    gridSubdiv = [subdivType isEqualToString:@"grid"];
    NSString *texProjStr = [desc stringForKey:@"texprojection" default:nil];
    texProj = TextureProjectionNone;
    if ([texProjStr isEqualToString:@"texprojectiontanplane"])
        texProj = TextureProjectionTanPlane;
    else if ([texProjStr isEqualToString:@"texprojectionscreen"])
        texProj = TextureProjectionScreen;
    
    // Note: From Android version
//    centered = dict.getBool(MaplyVecCentered,true);
//    if (dict.hasField("veccenterx") && dict.hasField("veccentery"))
//    {
//        vecCenterSet = true;
//        vecCenter.x() = dict.getDouble("veccenterx");
//        vecCenter.x() = dict.getDouble("veccentery");
//    }

}

}
