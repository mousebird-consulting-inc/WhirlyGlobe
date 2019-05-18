/*
 *  BasicDrawableBuilderMTL.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/16/19.
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

#import "BasicDrawableBuilderMTL.h"

namespace WhirlyKit
{

BasicDrawableBuilderMTL::BasicDrawableBuilderMTL(const std::string &name)
    : BasicDrawableBuilder(name)
{
}
    
BasicDrawableBuilderMTL::~BasicDrawableBuilderMTL()
{
}

int BasicDrawableBuilderMTL::addAttribute(BDAttributeDataType dataType,StringIdentity nameID,int numThings)
{
    // TODO: Implement
    
    return -1;
}

BasicDrawable *BasicDrawableBuilderMTL::getDrawable()
{
    // TODO: Implement
    return NULL;
}
    
}
