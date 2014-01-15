/*
 *  Platform.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 12/13/13.
 *  Copyright 2011-2013 mousebird consulting
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

#import <ctime>
#import <vector>
#import "WhirlyTypes.h"

namespace WhirlyKit
{

// Wrapper for platform specific time function
extern TimeInterval TimeGetCurrent();
    
// Retina vs. not on iOS.  Always 1.0 on Android
extern float DeviceScreenScale();

// Base class for NSData replacement (sort of).
// In this state it's read only
class RawData
{
public:
    RawData();
    virtual ~RawData();
    // Return a pointer to the raw data we're keeping
    virtual const unsigned char *getRawData() const = 0;
    // Length of the buffer
    virtual unsigned long getLen() const = 0;
    
protected:
};
    
// Read only version that wraps a random collection of bytes
class RawDataWrapper : public RawData
{
public:
    RawDataWrapper(const void *data,unsigned long dataLen,bool freeWhenDone);
    virtual ~RawDataWrapper();
    // Return a pointer to the raw data we're keeping
    virtual const unsigned char *getRawData() const;
    // Length of the raw data collected thus far
    unsigned long getLen() const;

protected:
    bool freeWhenDone;
    const unsigned char *data;
    unsigned int len;
};

// You can add data to this one as needed
class MutableRawData : public RawData
{
public:
    MutableRawData();
    virtual ~MutableRawData();
    // Return a pointer to the raw data we're keeping
    virtual const unsigned char *getRawData() const;
    // Length of the raw data collected thus far
    virtual unsigned long getLen() const;
    
protected:
    std::vector<unsigned char> data;
};
    
}
