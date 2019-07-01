/*
 *  RawData_NSData.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/23/19.
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
#import "RawData.h"

namespace WhirlyKit {

/** Wrapper around an NSData object.
 
    This wrappper treats an NSData as a RawData object for internal
    WhirlyKit processing.
  */
class RawNSDataReader : public RawData
{
public:
    RawNSDataReader(NSData *data);
    ~RawNSDataReader();
    
    // Return a pointer to the raw data we're keeping
    virtual const unsigned char *getRawData() const;
    // Length of the buffer
    virtual unsigned long getLen() const;
    
    // Return the NSData object
    NSData *getData();

protected:
    NSData *data;
};
    
typedef std::shared_ptr<RawNSDataReader> RawNSDataReaderRef;

}
