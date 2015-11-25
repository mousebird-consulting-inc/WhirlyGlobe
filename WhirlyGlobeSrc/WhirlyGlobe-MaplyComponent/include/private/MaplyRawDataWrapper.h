/*
 *  MaplyRawDataWrapper.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/26/14.
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
class RawNSDataWrapper : public RawData
{
public:
    RawNSDataWrapper(NSData *data);
    
    // Return a pointer to the raw data we're keeping
    virtual const unsigned char *getRawData() const;
    // Length of the raw data collected thus far
    unsigned long getLen() const;

    
protected:
    NSData *data;
};
}
