/*
 *  SingleLabel_iOS.h
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

#import <UIKit/UIKit.h>
#import "LabelManager.h"

namespace WhirlyKit
{

/// SingleLabel variant that knows iOS rendering
class SingleLabel_iOS: public SingleLabel
{
public:
    // Used to build the drawable string on specific platforms
    virtual std::vector<DrawableString *> generateDrawableStrings(PlatformThreadInfo *threadInfo,const LabelInfo *,FontTextureManager *fontTexManager,float &lineHeight,ChangeSet &changes) override;
    
    // Pass this around as an NSString
    NSString *text;
};
typedef std::shared_ptr<SingleLabel_iOS> SingleLabel_iOSRef;
    
/// LabelInfo variant knows UIFont and related stuff
class LabelInfo_iOS: public LabelInfo
{
public:
    LabelInfo_iOS(NSDictionary *iosDict,const Dictionary &dict,bool screenObject);
    LabelInfo_iOS(UIFont *font,bool screenObject);
    
    UIFont *font;
};
typedef std::shared_ptr<LabelInfo_iOS> LabelInfo_iOSRef;
    
}
