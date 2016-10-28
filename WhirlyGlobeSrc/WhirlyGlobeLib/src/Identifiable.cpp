/*
 *  Identifiable.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/7/11.
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

#import <mutex>
#import "Identifiable.h"

namespace WhirlyKit
{
	
static unsigned long curId = 0;
static std::mutex identMutex;

Identifiable::Identifiable()
{ 
    identMutex.lock();
    myId = ++curId; 
    identMutex.unlock();
}
	
SimpleIdentity Identifiable::genId()
{
    identMutex.lock();
    SimpleIdentity newID = ++curId;
    identMutex.unlock();
    
    return newID;
}

}
