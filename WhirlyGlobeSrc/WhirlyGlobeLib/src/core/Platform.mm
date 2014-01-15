/*
 *  Platform.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 12/13/14.
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

#import <UIKit/UIKit.h>
#import "Platform.h"

namespace WhirlyKit
{

TimeInterval TimeGetCurrent()
{
    return CFAbsoluteTimeGetCurrent();
}
    
float DeviceScreenScale()
{
    return [UIScreen mainScreen].scale;
}
    
RawData::RawData()
{
}
    
RawData::~RawData()
{
}
    
RawDataWrapper::RawDataWrapper(const void *inData,unsigned long dataLen,bool freeWhenDone)
    : len(dataLen), freeWhenDone(freeWhenDone)
{
    data = (const unsigned char *)inData;
}
    
RawDataWrapper::~RawDataWrapper()
{
    if (freeWhenDone)
        delete data;
    data = NULL;
}
    
const unsigned char *RawDataWrapper::getRawData() const
{
    return data;
}
    
unsigned long RawDataWrapper::getLen() const
{
    return len;
}
    
MutableRawData::MutableRawData()
{
}
    
MutableRawData::~MutableRawData()
{
}
    
const unsigned char *MutableRawData::getRawData() const
{
    if (data.empty())
        return NULL;
    return &data[0];
}
    
unsigned long MutableRawData::getLen() const
{
    return data.size();
}

}
