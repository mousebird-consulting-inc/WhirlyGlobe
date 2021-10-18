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
std::vector<std::unique_ptr<DrawableString>> SingleLabel_iOS::generateDrawableStrings(
        PlatformThreadInfo *threadInfo, const LabelInfo *inLabelInfo,
        const FontTextureManagerRef &inFontTexManager,float &lineHeight,ChangeSet &changes)
{
    auto fontTexManager = dynamic_cast<FontTextureManager_iOS*>(inFontTexManager.get());
    const LabelInfo_iOS *labelInfo = (LabelInfo_iOS *)inLabelInfo;

    std::vector<std::unique_ptr<DrawableString>> drawStrs;

    if (!fontTexManager || !labelInfo)
    {
        return drawStrs;
    }

    NSArray *strings = [text componentsSeparatedByString:@"\n"];
    if (strings.count == 0)
    {
        return drawStrs;
    }

    // May need the line height for multi-line labels
    lineHeight = labelInfo->font.lineHeight;
    if (labelInfo->lineHeight > 0.0)
        lineHeight = labelInfo->lineHeight;
    if (infoOverride && infoOverride->lineHeight > 0.0)
        lineHeight = infoOverride->lineHeight;
    
    float offset = 0.0f;
    for (NSString *text in strings)
    {
        // Build the attributed string
        NSMutableAttributedString *attrStr = [[NSMutableAttributedString alloc] initWithString:text];
        NSInteger strLen = [attrStr length];
        [attrStr addAttribute:NSFontAttributeName value:labelInfo->font range:NSMakeRange(0, strLen)];
        if (labelInfo->outlineSize > 0.0)
        {
            UIColor *outlineColor = [UIColor colorWithRed:labelInfo->outlineColor.r/255.0f
                                                    green:labelInfo->outlineColor.g/255.0f
                                                     blue:labelInfo->outlineColor.b/255.0f
                                                    alpha:labelInfo->outlineColor.a/255.0f];
            UIColor *textColor = [UIColor colorWithRed:labelInfo->textColor.r/255.0f
                                                 green:labelInfo->textColor.g/255.0f
                                                  blue:labelInfo->textColor.b/255.0f
                                                 alpha:labelInfo->textColor.a/255.0f];
            [attrStr addAttribute:kOutlineAttributeSize
                            value:[NSNumber numberWithFloat:labelInfo->outlineSize]
                            range:NSMakeRange(0, strLen)];
            [attrStr addAttribute:kOutlineAttributeColor
                            value:outlineColor range:NSMakeRange(0, strLen)];
            [attrStr addAttribute:NSForegroundColorAttributeName
                            value:textColor
                            range:NSMakeRange(0, strLen)];
        }

        if (auto drawStr = fontTexManager->addString(threadInfo, attrStr, changes))
        {
            // Modify the MBR if this is a multi-line label
            drawStr->mbr.ll().y() += offset;
            drawStr->mbr.ur().y() += offset;

            drawStrs.push_back(std::move(drawStr));
        }
        offset += lineHeight;
    }
    
    return drawStrs;
}

}
