/*
 *  SingleLabelAndroid.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
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

#import "SingleLabel_Android.h"
#import "FontTextureManager_Android.h"
#import "LabelInfo_Android.h"

namespace WhirlyKit
{

std::vector<DrawableString *> SingleLabelAndroid::generateDrawableStrings(PlatformThreadInfo *inThreadInfo,const LabelInfo *inLabelInfo,FontTextureManager *inFontTexManager,float &lineHeight,ChangeSet &changes)
{
	FontTextureManager_Android *fontTexManager = (FontTextureManager_Android *)inFontTexManager;
	const LabelInfoAndroid *labelInfo = (LabelInfoAndroid *)inLabelInfo;
    PlatformInfo_Android *threadInfo = (PlatformInfo_Android *)inThreadInfo;

    // May need the line height for multi-line labels
    lineHeight = labelInfo->lineHeight;

    std::vector<DrawableString *> drawStrs;
    int whichLine = 0;
    for (std::vector<int> &codePoints : codePointsLines)
    {
        DrawableString *drawStr = fontTexManager->addString(threadInfo,codePoints,labelInfo,changes);
        if (drawStr)
            drawStrs.push_back(drawStr);

        // Modify the MBR if this is a multi-line label
        if (whichLine > 0) {
            drawStr->mbr.ll().y() += lineHeight * whichLine;
            drawStr->mbr.ur().y() += lineHeight * whichLine;
        }
        whichLine++;
    }



    return drawStrs;
}

}
