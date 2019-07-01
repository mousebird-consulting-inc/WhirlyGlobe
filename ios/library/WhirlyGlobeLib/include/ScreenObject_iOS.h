/*
 *  ScreenObject_iOS.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/8/19.
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

#import "WhirlyGlobe.h"
#import <Foundation/Foundation.h>

namespace WhirlyKit
{

// iOS version allows for UIImages
class SimplePoly_iOS : public SimplePoly {
public:
    SimplePoly_iOS();
    
    id texture;
};
    
typedef std::shared_ptr<SimplePoly_iOS> SimplePoly_iOSRef;
    
// iOS version of the string is an NSAttributedString
class StringWrapper_iOS : public StringWrapper {
public:
    StringWrapper_iOS();
    
    NSAttributedString *str;
};
    
typedef std::shared_ptr<StringWrapper_iOS> StringWrapper_iOSRef;
    
}
