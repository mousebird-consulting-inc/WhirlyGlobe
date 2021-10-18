/*  SingleLabelAndroid.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
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

#import "SingleLabel_Android.h"
#import "FontTextureManager_Android.h"
#import "LabelInfo_Android.h"

namespace WhirlyKit
{

std::vector<std::unique_ptr<DrawableString>> SingleLabelAndroid::generateDrawableStrings(
        PlatformThreadInfo *inThreadInfo,
        const LabelInfo *inLabelInfo,
        const FontTextureManagerRef &inFontTexManager,
        float &lineHeight,
        ChangeSet &changes)
{
	const auto fontTexManager = dynamic_cast<FontTextureManager_Android*>(inFontTexManager.get());
	if (!fontTexManager)
    {
	    return {};
    }

	auto labelInfo = (const LabelInfoAndroid *)inLabelInfo;
    auto threadInfo = (PlatformInfo_Android *)inThreadInfo;

    // May need the line height for multi-line labels
    lineHeight = labelInfo->lineHeight;

    std::vector<std::unique_ptr<DrawableString>> drawStrs;
    drawStrs.reserve(codePointsLines.size());

    float offset = 0.0f;
    for (const auto &codePoints : codePointsLines)
    {
        if (auto drawStr = fontTexManager->addString(threadInfo,codePoints,labelInfo,changes))
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
