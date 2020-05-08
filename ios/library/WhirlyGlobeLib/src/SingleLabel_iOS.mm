/*
 *  SingleLabel_iOS.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/4/19.
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

#import "SingleLabel_iOS.h"
#import "FontTextureManager_iOS.h"

namespace WhirlyKit
{
    
LabelInfo_iOS::LabelInfo_iOS(NSDictionary *iosDict,const Dictionary &dict,bool screenObject)
    : LabelInfo(dict,screenObject)
{
    font = [iosDict objectForKey:@"font"];
    fontPointSize = [font pointSize];
}

LabelInfo_iOS::LabelInfo_iOS(UIFont *font,bool screenObject)
: font(font),LabelInfo(screenObject)
{
    fontPointSize = [font pointSize];
}

    
// Used to build the drawable string on specific platforms
std::vector<DrawableString *> SingleLabel_iOS::generateDrawableStrings(PlatformThreadInfo *threadInfo,const LabelInfo *inLabelInfo,FontTextureManager *inFontTexManager,float &lineHeight,ChangeSet &changes)
{
    FontTextureManager_iOS *fontTexManager = (FontTextureManager_iOS *)inFontTexManager;
    const LabelInfo_iOS *labelInfo = (LabelInfo_iOS *)inLabelInfo;

    NSArray *strings = [text componentsSeparatedByString:@"\n"];
    std::vector<DrawableString *> drawStrs;

    // May need the line height for multi-line labels
    lineHeight = labelInfo->font.lineHeight;
    if (labelInfo->lineHeight > 0.0)
        lineHeight = labelInfo->lineHeight;
    if (infoOverride && infoOverride->lineHeight > 0.0)
        lineHeight = infoOverride->lineHeight;
    
    int whichLine=0;
    for (NSString *text in strings) {
        // Build the attributed string
        NSMutableAttributedString *attrStr = [[NSMutableAttributedString alloc] initWithString:text];
        NSInteger strLen = [attrStr length];
        [attrStr addAttribute:NSFontAttributeName value:labelInfo->font range:NSMakeRange(0, strLen)];
        if (labelInfo->outlineSize > 0.0)
        {
            UIColor *outlineColor = [UIColor colorWithRed:labelInfo->outlineColor.r/255.0 green:labelInfo->outlineColor.g/255.0 blue:labelInfo->outlineColor.b/255.0 alpha:labelInfo->outlineColor.a/255.0];
            UIColor *textColor = [UIColor colorWithRed:labelInfo->textColor.r/255.0 green:labelInfo->textColor.g/255.0 blue:labelInfo->textColor.b/255.0 alpha:labelInfo->textColor.a/255.0];
            [attrStr addAttribute:kOutlineAttributeSize value:[NSNumber numberWithFloat:labelInfo->outlineSize] range:NSMakeRange(0, strLen)];
            [attrStr addAttribute:kOutlineAttributeColor value:outlineColor range:NSMakeRange(0, strLen)];
            [attrStr addAttribute:NSForegroundColorAttributeName value:textColor range:NSMakeRange(0, strLen)];
        }

        DrawableString *drawStr = fontTexManager->addString(threadInfo, attrStr, changes);
        if (!drawStr)
            continue;
        
        // Modify the MBR if this is a multi-line label
        if (whichLine > 0) {
            drawStr->mbr.ll().y() += lineHeight * whichLine;
            drawStr->mbr.ur().y() += lineHeight * whichLine;
        }
        
        drawStrs.push_back(drawStr);
        whichLine++;
    }
    
    return drawStrs;
}
    
}
