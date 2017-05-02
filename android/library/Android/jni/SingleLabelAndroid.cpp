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

#import "SingleLabelAndroid.h"
#import "FontTextureManagerAndroid.h"
#import "LabelInfoAndroid.h"

namespace WhirlyKit
{

std::vector<DrawableString *> SingleLabelAndroid::generateDrawableStrings(const LabelInfo *inLabelInfo,FontTextureManager *inFontTexManager,ChangeSet &changes)
{
	FontTextureManagerAndroid *fontTexManager = (FontTextureManagerAndroid *)inFontTexManager;
	const LabelInfoAndroid *labelInfo = (LabelInfoAndroid *)inLabelInfo;

    std::vector<DrawableString *> drawStrs;
    for (std::vector<int> &codePoints : codePointsLines)
    {
        drawStrs.push_back(fontTexManager->addString(labelInfo->env,codePoints,labelInfo->labelInfoObj,changes));
    }
    
    return drawStrs;
}

}
