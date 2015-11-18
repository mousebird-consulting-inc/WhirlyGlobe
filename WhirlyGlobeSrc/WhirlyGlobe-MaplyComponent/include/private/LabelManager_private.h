/*
 *  LabelManager_private.h
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

#import <WhirlyGlobe.h>

namespace WhirlyKit
{
    
/// Subclass of LabelInfo for iOS
class LabelInfoiOS : public LabelInfo
{
public:
    // Font to use for a given set of labels
    UIFont *font;
};

/// Subclass of SingleLabel for iOS
class SingleLabeliOS : public SingleLabel
{
public:
    /// Text for this label
    NSString *text;
    
protected:
    // Generates a drawable string (e.g. geometry) for the iOS platform
    DrawableString *generateDrawableString(const LabelInfo *,FontTextureManager *fontTexManager,ChangeSet &changes);
};

}
