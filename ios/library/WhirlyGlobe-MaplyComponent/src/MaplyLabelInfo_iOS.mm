/*
 *  MaplyLabelInfo_iOS_private.h
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
#import "MaplyLabelInfo_iOS_private.h"
#import "UIColor+Stuff.h"
#import "NSDictionary+Stuff.h"

namespace WhirlyKit {
    
LabelInfo_iOS::LabelInfo_iOS(NSDictionary *desc)
{
    BaseInfoSetup(*this,desc);
    
    UIColor *theTextColor = [desc objectForKey:@"textColor" checkType:[UIColor class] default:[UIColor whiteColor]];
    if (theTextColor)
        textColor = [theTextColor asRGBAColor];
    UIColor *theBackColor = [desc objectForKey:@"backgroundColor" checkType:[UIColor class] default:[UIColor clearColor]];
    if (theBackColor)
        backColor = [theBackColor asRGBAColor];
    // Note: Change this
//    self.font = [desc objectForKey:@"font" checkType:[UIFont class] default:[UIFont systemFontOfSize:32.0]];
    screenObject = [desc boolForKey:@"screen" default:false];
    layoutEngine = [desc boolForKey:@"layout" default:false];
    layoutImportance = [desc floatForKey:@"layoutImportance" default:0.0];
    width = [desc floatForKey:@"width" default:0.0];
    height = [desc floatForKey:@"height" default:(screenObject ? 16.0 : 0.001)];
    NSString *labelJustifyStr = [desc stringForKey:@"justify" default:@"middle"];
    NSString *textJustifyStr = [desc stringForKey:@"textjustify" default:@"left"];
    UIColor *theShadowColor = [desc objectForKey:@"shadowColor"];
    if (theShadowColor)
        shadowColor = [theShadowColor asRGBAColor];
    shadowSize = [desc floatForKey:@"shadowSize" default:0.0];
    outlineSize = [desc floatForKey:@"outlineSize" default:0.0];
    UIColor *theOutlineColor = [desc objectForKey:@"outlineColor" checkType:[UIColor class] default:[UIColor blackColor]];
    if (theOutlineColor)
        outlineColor = [theOutlineColor asRGBAColor];
    if (![labelJustifyStr compare:@"middle"])
        labelJustify = WhirlyKitLabelMiddle;
    else {
        if (![labelJustifyStr compare:@"left"])
            labelJustify = WhirlyKitLabelLeft;
        else {
            if (![labelJustifyStr compare:@"right"])
                labelJustify = WhirlyKitLabelRight;
        }
    }
    if (![textJustifyStr compare:@"center"])
        textJustify = WhirlyKitTextCenter;
    else {
        if (![textJustifyStr compare:@"left"])
            textJustify = WhirlyKitTextLeft;
        else {
            if (![textJustifyStr compare:@"right"])
                textJustify = WhirlyKitTextRight;
        }
    }
}
    
}
