/*
 *  BaseInfo.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/6/15.
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

#import <math.h>
#import <set>
#import <map>
#import "Identifiable.h"
#import "WhirlyVector.h"
#import "Dictionary.h"

namespace WhirlyKit
{
class BasicDrawable;
class BasicDrawableInstance;

/** Object use as the base for parsing description dictionaries.
 */
class BaseInfo
{
public:
    BaseInfo();
    /// Construct with a dictionary
    BaseInfo(const Dictionary &dict);
    
    /// Set the various parameters on a basic drawable
    void setupBasicDrawable(BasicDrawable *drawable) const;

    /// Set the various parameters on a basic drawable instance
    // Note: Porting
//    void setupBasicDrawableInstance(BasicDrawableInstance *drawable);
    
    double minVis,maxVis;
    double minVisBand,maxVisBand;
    double minViewerDist,maxViewerDist;
    Point3d viewerCenter;
    double drawOffset;
    int drawPriority;
    bool enable;
    double fade;
    double fadeIn;
    double fadeOut;
    TimeInterval fadeOutTime;
    TimeInterval startEnable,endEnable;
    SimpleIdentity programID;
};

}
