/*
 *  RawData_NSData.mm
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

#import "RawData_NSData.h"

namespace WhirlyKit {
    
RawNSDataReader::RawNSDataReader(NSData *data)
: data(data)
{
}

RawNSDataReader::~RawNSDataReader()
{
    data = nil;
}
    
const unsigned char *RawNSDataReader::getRawData() const
{
    return (unsigned char *)[data bytes];
}

unsigned long RawNSDataReader::getLen() const
{
    return [data length];
}
    
NSData *RawNSDataReader::getData()
{
    return data;
}

}
