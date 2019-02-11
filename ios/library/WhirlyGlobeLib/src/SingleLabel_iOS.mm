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
}
    
// Used to build the drawable string on specific platforms
std::vector<DrawableString *> SingleLabel_iOS::generateDrawableStrings(const LabelInfo *inLabelInfo,FontTextureManager *inFontTexManager,ChangeSet &changes)
{
    FontTextureManager_iOS *fontTexManager = (FontTextureManager_iOS *)inFontTexManager;
    const LabelInfo_iOS *labelInfo = (LabelInfo_iOS *)inLabelInfo;

    // Note: Need the multi-line logic in here somewhere

    // Build the attributed string
    NSString *basicString = [NSString stringWithFormat:@"%s",text.c_str()];
    NSMutableAttributedString *attrStr = [[NSMutableAttributedString alloc] initWithString:basicString];
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

    std::vector<DrawableString *> drawStrs;
    DrawableString *drawStr = fontTexManager->addString(attrStr, changes);
    if (drawStr)
        drawStrs.push_back(drawStr);
    
    return drawStrs;
}
    
}
