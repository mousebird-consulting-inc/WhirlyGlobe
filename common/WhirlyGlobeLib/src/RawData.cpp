/*  RawData.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/15/14.
 *  Copyright 2011-2021 mousebird consulting
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
 */

#include <cstdlib>
#include <string>
#include <cstring>
#include <utility>
#import "RawData.h"

namespace WhirlyKit
{

static void defaultFree(const void* p) { delete[] (char*)p; }
using FreeFunc = std::function<void(const void*)>;

RawDataWrapper::RawDataWrapper(const void *inData,unsigned long dataLen,bool freeWhenDone) :
    RawDataWrapper(inData, dataLen, freeWhenDone ? FreeFunc(&defaultFree) : nullptr)
{
}

RawDataWrapper::RawDataWrapper(const void *inData,unsigned long dataLen, FreeFunc freer) :
    data((const unsigned char *)inData),
    len(dataLen),
    freeFunc(std::move(freer))
{
}

RawDataWrapper::RawDataWrapper(RawDataWrapper &&other) noexcept :
    data(other.data),
    len(other.len),
    freeFunc(std::move(other.freeFunc))
{
    other.data = nullptr;
    other.len = 0;
}

RawDataWrapper::~RawDataWrapper()
{
    if (data && freeFunc)
    {
        freeFunc(data);
    }
    data = nullptr;
}

RawDataReader::RawDataReader(const RawData *rawData) :
    rawData(rawData),
    pos(0)
{
}

bool RawDataReader::done() const
{
    return (pos >= rawData->getLen());
}

bool RawDataReader::getInt(int &val)
{
    const size_t dataSize = sizeof(int);
    if (pos+dataSize > rawData->getLen())
        return false;
    val = *((int *)(rawData->getRawData()+pos));
    pos += dataSize;
    
    return true;
}

bool RawDataReader::getInt64(int64_t &val)
{
    const size_t dataSize = sizeof(int64_t);
    if (pos+dataSize > rawData->getLen())
        return false;
    val = *((int *)(rawData->getRawData()+pos));
    pos += dataSize;

    return true;
}

bool RawDataReader::getDouble(double &val)
{
    const size_t dataSize = sizeof(double);
    if (pos+dataSize > rawData->getLen())
        return false;
    val = *((int *)(rawData->getRawData()+pos));
    pos += dataSize;
    
    return true;
}

bool RawDataReader::getString(std::string &str)
{
    int dataLen;
    if (!getInt(dataLen))
        return false;
    if (pos+dataLen > rawData->getLen())
        return false;
    str = std::string((char *)(rawData->getRawData()+pos), dataLen);
    
    pos += dataLen;
    return true;
}


MutableRawData::MutableRawData(void *inData,unsigned int size)
{
    data.resize(size);
    
    memcpy(&data[0], inData, size);
}
    
MutableRawData::MutableRawData(unsigned int size)
{
    data.resize(size);
    memset(&data[0], 0, size);
}

const unsigned char *MutableRawData::getRawData() const
{
    return data.empty() ? nullptr : &data[0];
}

void MutableRawData::addInt(int iVal)
{
    const size_t len = sizeof(int);
    data.resize(data.size()+len);
    memcpy(&data[data.size()-len], &iVal, len);
}

void MutableRawData::addInt64(int64_t iVal)
{
    const size_t len = sizeof(int64_t);
    data.resize(data.size()+len);
    memcpy(&data[data.size()-len], &iVal, len);
}

void MutableRawData::addDouble(double dVal)
{
    const size_t len = sizeof(double);
    data.resize(data.size()+len);
    memcpy(&data[data.size()-len], &dVal, len);
}

void MutableRawData::addString(const std::string &str)
{
    const size_t len = str.length();
    const size_t extra = (4 - len % 4) % 4;
    addInt(len+extra);
    const size_t start = data.size();
    data.resize(data.size()+len+extra);
    memcpy(&data[start], str.c_str(), len);
    memset(&data[start+len], 0, extra);
}

RawDataWrapper *RawDataFromFile(FILE *fp,unsigned int dataLen)
{
    auto *data = new unsigned char[dataLen];

    if (fread(data, dataLen, 1, fp) != 1)
    {
        delete[] data;
        return nullptr;
    }
    
    return new RawDataWrapper(data,dataLen,true);
}

}
