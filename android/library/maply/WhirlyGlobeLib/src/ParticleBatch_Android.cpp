/*
 *  ParticleBatch_Android.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/13/19.
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

 #import "ParticleBatch_Android.h"

namespace WhirlyKit
{

ParticleBatch_Android::ParticleBatch_Android()
: partSys(NULL), baseTime(0.0)
{
}

ParticleBatch_Android::~ParticleBatch_Android()
{
    for (auto data : attrData)
        free((void *)data);
    attrData.clear();
}

bool ParticleBatch_Android::addAttributeDataFloat(const std::string &name,const float *data,int len)
{
    auto nameID = StringIndexer::getStringID(name);

    for (auto attr : partSys->vertAttrs)
    {
        if (attr.nameID == nameID) {
            int size = len * sizeof(float);
            if (size != attr.size() * batchSize)
                break;
            unsigned char *copyData = (unsigned char *)malloc(size);
            memcpy(copyData,data,size);
            attrData.push_back(copyData);
            return true;
        }
    }

    return false;
}

bool ParticleBatch_Android::addAttributeDataChar(const std::string &name,const char *data,int len)
{
    auto nameID = StringIndexer::getStringID(name);

    for (auto attr : partSys->vertAttrs)
    {
        if (attr.nameID == nameID) {
            int size = len;
            if (size != attr.size() * batchSize)
                break;
            unsigned char *copyData = (unsigned char *)malloc(size);
            memcpy(copyData,data,size);
            attrData.push_back(copyData);
            return true;
        }
    }

    return false;
}

}
