/*
 *  LabelManager.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/20/14.
 *  Copyright 2011-2015 mousebird consulting
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

#import <UIKit/UIKit.h>
#import <CoreText/CoreText.h>
#import "UIColor+Stuff.h"
#import "LabelManager_private.h"
#import "FontTextureManageriOS.h"

namespace WhirlyKit
{

DrawableString *SingleLabeliOS::generateDrawableString(const LabelInfo *inLabelInfo,FontTextureManager *inFontTexManager,ChangeSet &changes)
{
    LabelInfoiOS *labelInfo = (LabelInfoiOS *)inLabelInfo;
    FontTextureManageriOS *fontTexManager = (FontTextureManageriOS *)inFontTexManager;
    
    RGBAColor theTextColor = labelInfo->textColor;
    RGBAColor theOutlineColor = labelInfo->outlineColor;
    float theOutlineSize = labelInfo->outlineSize;
    UIFont *theFont = labelInfo->font;
    if (desc.numFields() > 0)
    {
        theTextColor = desc.getColor(MaplyTextColor,theTextColor);
        theOutlineSize = desc.getDouble(MaplyTextOutlineSize,theOutlineSize);
        theOutlineColor = desc.getColor(MaplyTextOutlineColor,theOutlineColor);
        // Note: Porting.  Should look for font in Dictionary
    }
    
    // Build the attributed string
    NSMutableAttributedString *attrStr = [[NSMutableAttributedString alloc] initWithString:text];
    NSInteger strLen = [attrStr length];
    [attrStr addAttribute:NSFontAttributeName value:theFont range:NSMakeRange(0, strLen)];
    if (theOutlineSize > 0.0)
    {
        [attrStr addAttribute:kOutlineAttributeSize value:[NSNumber numberWithFloat:theOutlineSize] range:NSMakeRange(0, strLen)];
        [attrStr addAttribute:kOutlineAttributeColor value:[UIColor colorFromRGBA:theOutlineColor] range:NSMakeRange(0, strLen)];
        [attrStr addAttribute:NSForegroundColorAttributeName value:[UIColor colorFromRGBA:theTextColor] range:NSMakeRange(0, strLen)];
    }
    
    return fontTexManager->addString(attrStr,changes);
}
    
}