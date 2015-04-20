/*
 *  RawData.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/15/14.
 *  Copyright 2011-2014 mousebird consulting
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

#include <stdlib.h>
#include <string>
#include <unistd.h>
#import "RawData.h"

namespace WhirlyKit
{

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
    
RawDataReader::RawDataReader(const RawData *rawData)
: rawData(rawData), pos(0)
{
}

bool RawDataReader::done()
{
    return (pos >= rawData->getLen());
}

bool RawDataReader::getInt(int &val)
{
    size_t dataSize = sizeof(int);
    if (pos+dataSize > rawData->getLen())
        return false;
    val = *((int *)(rawData->getRawData()+pos));
    pos += dataSize;
    
    return true;
}

bool RawDataReader::getDouble(double &val)
{
    size_t dataSize = sizeof(double);
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


MutableRawData::MutableRawData()
{
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

void MutableRawData::addInt(int iVal)
{
    size_t len = sizeof(int);
    data.resize(data.size()+len);
    memcpy(&data[data.size()-len], &iVal, len);
}

void MutableRawData::addDouble(double dVal)
{
    size_t len = sizeof(double);
    data.resize(data.size()+len);
    memcpy(&data[data.size()-len], &dVal, len);
}

void MutableRawData::addString(const std::string &str)
{
    size_t len = str.length();
    size_t extra = (4 - len % 4) % 4;
    addInt(len+extra);
    size_t start = data.size();
    data.resize(data.size()+len+extra);
    memcpy(&data[start], str.c_str(), len);
    memset(&data[start+len], 0, extra);
}

    
RawDataWrapper *RawDataFromFile(FILE *fp,unsigned int dataLen)
{
    unsigned char *data = (unsigned char *)malloc((size_t)dataLen);

    if (fread(data, dataLen, 1, fp) != 1)
    {
        free(data);
        return NULL;
    }
    
    return new RawDataWrapper(data,dataLen,true);
}

}
