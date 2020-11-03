/*
 *  Dictionary.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 12/16/13.
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

#import <sstream>
#import "DictionaryC.h"

namespace WhirlyKit
{

MutableDictionaryC::MutableDictionaryC()
    : stringMap(20)     // ?
    , valueMap(20)
{
}

MutableDictionaryC::MutableDictionaryC(int capacity)
    : stringMap(capacity)
    , valueMap(capacity)
{
}

MutableDictionaryC::MutableDictionaryC(const MutableDictionaryC &that)
    : intVals(that.intVals)
    , int64Vals(that.int64Vals)
    , dVals(that.dVals)
    , stringVals(that.stringVals)
    , arrayVals(that.arrayVals)
    , dictVals(that.dictVals)
    , stringMap(that.stringMap)
    , valueMap(that.valueMap)
{
}

MutableDictionaryC::MutableDictionaryC(MutableDictionaryC &&that)
    : intVals(std::move(that.intVals))
    , int64Vals(std::move(that.int64Vals))
    , dVals(std::move(that.dVals))
    , stringVals(std::move(that.stringVals))
    , arrayVals(std::move(that.arrayVals))
    , dictVals(std::move(that.dictVals))
    , stringMap(std::move(that.stringMap))
    , valueMap(std::move(that.valueMap))
{
}

MutableDictionaryC::~MutableDictionaryC()
{
    clear();
}

//bool MutableDictionaryC::parseJSON(const std::string jsonString)
//{
//    json_string json = jsonString;
//
//    JSONNode topNode = libjson::parse(json);
//    return parseJSONNode(topNode);
//}
//
//MutableDictionaryC::ValueRef MutableDictionaryC::parseJSONValue(JSONNode::iterator &nodeIt)
//{
//    switch (nodeIt->type()) {
//        case JSON_NULL:
//            break;
//        case JSON_STRING:
//            return ValueRef(new StringValue(nodeIt->as_string()));
//            break;
//        case JSON_NUMBER:
//            return ValueRef(new DoubleValue(nodeIt->as_float()));
//            break;
//        case JSON_BOOL:
//            return ValueRef(new IntValue(nodeIt->as_bool()));
//            break;
//        case JSON_ARRAY:
//        {
//            auto nodes = nodeIt->as_array();
//            std::vector<ValueRef> values;
//            for (JSONNode::iterator arrNodeIt = nodes.begin(); arrNodeIt != nodes.end(); ++arrNodeIt) {
//                values.push_back(parseJSONValue(arrNodeIt));
//            }
//            return ArrayValueRef(new ArrayValue(values));
//        }
//            break;
//        case JSON_NODE:
//        {
//            MutableDictionaryCRef dict(new MutableDictionaryC());
//            auto node = nodeIt->as_node();
//            dict->parseJSONNode(node);
//            return DictionaryValueRef(new DictionaryValue(dict));
//        }
//            break;
//    }
//
//    return ValueRef();
//}
//
//bool MutableDictionaryC::parseJSONNode(JSONNode &node)
//{
//    for (JSONNode::iterator nodeIt = node.begin(); nodeIt != node.end(); ++nodeIt) {
//        auto name = nodeIt->name();
//        ValueRef val = parseJSONValue(nodeIt);
//        if (name.empty() || !val)
//            return false;
//        fields[name] = val;
//    }
//
//    return true;
//}

void MutableDictionaryC::clear()
{
    intVals.clear();
    int64Vals.clear();
    dVals.clear();
    stringVals.clear();
    arrayVals.clear();
    dictVals.clear();
    stringMap.clear();
    valueMap.clear();
}
    
MutableDictionaryC &MutableDictionaryC::operator = (const MutableDictionaryC &that)
{
    intVals = that.intVals;
    int64Vals = that.int64Vals;
    dVals = that.dVals;
    stringVals = that.stringVals;
    arrayVals = that.arrayVals;
    dictVals = that.dictVals;
    stringMap = that.stringMap;
    valueMap = that.valueMap;
    
    return *this;
}

MutableDictionaryC &MutableDictionaryC::operator = (MutableDictionaryC &&that)
{
    intVals = std::move(that.intVals);
    int64Vals = std::move(that.int64Vals);
    dVals = std::move(that.dVals);
    stringVals = std::move(that.stringVals);
    arrayVals = std::move(that.arrayVals);
    dictVals = std::move(that.dictVals);
    stringMap = std::move(that.stringMap);
    valueMap = std::move(that.valueMap);
    
    return *this;
}

//MutableDictionaryC::MutableDictionaryC(RawData *rawData)
//{
//    RawDataReader dataRead(rawData);
//    while (!dataRead.done())
//    {
//        int type;
//        if (!dataRead.getInt(type))
//            return;
//        std::string attrName;
//        if (!dataRead.getString(attrName))
//            return;
//        switch (type)
//        {
//            case DictTypeString:
//            {
//                std::string sVal;
//                if (!dataRead.getString(sVal))
//                    return;
//                setString(attrName, sVal);
//            }
//                break;
//            case DictTypeInt:
//            {
//                int iVal;
//                if (!dataRead.getInt(iVal))
//                    return;
//                setInt(attrName, iVal);
//            }
//                break;
//            case DictTypeDouble:
//            {
//                double dVal;
//                if (!dataRead.getDouble(dVal))
//                    return;
//                setDouble(attrName, dVal);
//            }
//                break;
//            default:
//                return;
//        }
//    }
//}
//    
//void MutableDictionaryC::asRawData(MutableRawData *rawData)
//{
//    for (FieldMap::iterator it = fields.begin(); it != fields.end(); ++it)
//    {
//        ValueRef val = it->second;
//        if (val->type() == DictTypeObject)
//            continue;
//        rawData->addInt(val->type());
//        rawData->addString(it->first);
//        switch (val->type())
//        {
//            case DictTypeString:
//            {
//                std::string str;
//                val->asString(str);
//                rawData->addString(str);
//            }
//                break;
//            case DictTypeInt:
//                rawData->addInt(val->asInt());
//                break;
//            case DictTypeDouble:
//                rawData->addDouble(val->asDouble());
//                break;
//            default:
//                throw 1;
//                break;
//        }
//    }
//}
        
bool MutableDictionaryC::hasField(const std::string &name) const
{
    const auto it = stringMap.find(name);
    return (it != stringMap.end()) && hasField(it->second);
}

bool MutableDictionaryC::hasField(unsigned int key) const
{
    const auto it = valueMap.find(key);
    return (it != valueMap.end());
}
    
DictionaryType MutableDictionaryC::getType(const std::string &name) const
{
    const auto it = stringMap.find(name);
    if (it == stringMap.end())
        return DictTypeNone;

    return getType(it->second);
}

DictionaryType MutableDictionaryC::getType(unsigned int key) const
{
    const auto it = valueMap.find(key);
    if (it == valueMap.end())
        return DictTypeNone;

    return it->second.type;
}

void MutableDictionaryC::removeField(const std::string &name)
{
    const auto it = stringMap.find(name);
    if (it == stringMap.end())
        return;
    removeField(it->second);
}

void MutableDictionaryC::removeField(unsigned int key)
{
    // TODO: WE're "leaking" (not actually) leaking space in the data arrays
    auto it = valueMap.find(key);
    if (it != valueMap.end())
        valueMap.erase(it);
}
    
int MutableDictionaryC::getInt(const std::string &name,int defVal) const
{
    const auto it = stringMap.find(name);
    if (it == stringMap.end())
        return defVal;

    return getInt(it->second,defVal);
}

int MutableDictionaryC::getInt(unsigned int key,int defVal) const
{
    const auto it = valueMap.find(key);
    if (it == valueMap.end())
        return defVal;
    
    const auto &val = it->second;
    switch (val.type) {
        case DictTypeInt:
            return intVals[val.entry];
        case DictTypeInt64:
            return int64Vals[val.entry];
        case DictTypeDouble:
            return (int)dVals[val.entry];
        // TODO: Maybe parse the string
        default:
            return defVal;
    }
}
    
SimpleIdentity MutableDictionaryC::getIdentity(const std::string &name) const
{
    const auto it = stringMap.find(name);
    if (it == stringMap.end())
        return EmptyIdentity;

    return getIdentity(it->second);
}

SimpleIdentity MutableDictionaryC::getIdentity(unsigned int key) const
{
    const auto it = valueMap.find(key);
    if (it == valueMap.end())
        return EmptyIdentity;
    
    const auto &val = it->second;
    switch (val.type) {
        case DictTypeInt:
            return intVals[val.entry];
        case DictTypeInt64:
        case DictTypeIdentity:
            return int64Vals[val.entry];
        case DictTypeDouble:
            return (int)dVals[val.entry];
        // TODO: Maybe parse the string
        default:
            return EmptyIdentity;
    }
}

int64_t MutableDictionaryC::getInt64(const std::string &name,int64_t defVal) const
{
    const auto it = stringMap.find(name);
    if (it == stringMap.end())
        return defVal;

    return getInt64(it->second,defVal);
}

int64_t MutableDictionaryC::getInt64(unsigned int key,int64_t defVal) const
{
    const auto it = valueMap.find(key);
    if (it == valueMap.end())
        return defVal;
    
    const auto &val = it->second;
    switch (val.type) {
        case DictTypeInt:
            return intVals[val.entry];
        case DictTypeInt64:
        case DictTypeIdentity:
            return int64Vals[val.entry];
        case DictTypeDouble:
            return (int)dVals[val.entry];
        // TODO: Maybe parse the string
        default:
            return 0;
    }
}

bool MutableDictionaryC::getBool(const std::string &name,bool defVal) const
{
    const auto it = stringMap.find(name);
    if (it == stringMap.end())
        return defVal;

    return getBool(it->second);
}

bool MutableDictionaryC::getBool(unsigned int key,bool defVal) const
{
    const auto it = valueMap.find(key);
    if (it == valueMap.end())
        return defVal;
    
    const auto &val = it->second;
    switch (val.type) {
        case DictTypeInt:
            return intVals[val.entry] != 0;
        case DictTypeInt64:
            return int64Vals[val.entry] != 0;
        // TODO: Maybe parse the string
        default:
            return defVal;
    }
}

RGBAColor MutableDictionaryC::getColor(const std::string &name,const RGBAColor &defVal) const
{
    const auto it = stringMap.find(name);
    if (it == stringMap.end())
        return defVal;

    return getColor(it->second,defVal);
}

RGBAColor MutableDictionaryC::getColor(unsigned int key,const RGBAColor &defVal) const
{
    const auto it = valueMap.find(key);
    if (it == valueMap.end())
        return defVal;

    const auto &val = it->second;
    switch (val.type)
    {
        case DictTypeString:
        {
            const std::string &str = stringVals[val.entry];
            // We're looking for a #RRGGBBAA
            if (str.length() < 1 || str[0] != '#')
                return defVal;
            
            int iVal = atoi(&str.c_str()[1]);
            RGBAColor ret;
            ret.b = iVal & 0xFF;
            ret.g = (iVal >> 8) & 0xFF;
            ret.r = (iVal >> 16) & 0xFF;
            ret.a = (iVal >> 24) & 0xFF;
            return ret;
        }
        case DictTypeInt:
        {
            int iVal = intVals[val.entry];
            RGBAColor ret;
            ret.b = iVal & 0xFF;
            ret.g = (iVal >> 8) & 0xFF;
            ret.r = (iVal >> 16) & 0xFF;
            ret.a = (iVal >> 24) & 0xFF;
            return ret;
        }
        // No idea what this means
        case DictTypeDouble:
        default:
            return defVal;
    }
    
    return defVal;
}
    
double MutableDictionaryC::getDouble(const std::string &name,double defVal) const
{
    const auto it = stringMap.find(name);
    if (it == stringMap.end())
        return defVal;

    return getDouble(it->second,defVal);
}

double MutableDictionaryC::getDouble(unsigned int key,double defVal) const
{
    const auto it = valueMap.find(key);
    if (it == valueMap.end())
        return defVal;
    
    const auto &val = it->second;
    switch (val.type) {
        case DictTypeInt:
            return intVals[val.entry];
        case DictTypeInt64:
        case DictTypeIdentity:
            return int64Vals[val.entry];
        case DictTypeDouble:
            return dVals[val.entry];
        // TODO: Maybe parse the string
        default:
            return 0;
    }
}

std::string MutableDictionaryC::getString(const std::string &name) const
{
    const auto it = stringMap.find(name);
    if (it == stringMap.end())
        return std::string();
    return getString(it->second);
}

std::string MutableDictionaryC::getString(unsigned int key) const
{
    const auto it = valueMap.find(key);
    if (it == valueMap.end())
        return std::string();
    auto const &val = it->second;
    switch (val.type) {
        case DictTypeString:
            return stringVals[val.entry];
        case DictTypeInt:
            return std::to_string(intVals[val.entry]);
        case DictTypeInt64:
        case DictTypeIdentity:
            return std::to_string(int64Vals[val.entry]);
        case DictTypeDouble:
            return std::to_string(dVals[val.entry]);
        case DictTypeNone:
        case DictTypeObject:
        case DictTypeDictionary:
        case DictTypeArray:
            return std::string();
    }
}

std::string MutableDictionaryC::getString(const std::string &name,const std::string &defVal) const
{
    const auto it = stringMap.find(name);
    if (it == stringMap.end())
        return defVal;
    return getString(it->second,defVal);
}

std::string MutableDictionaryC::getString(unsigned int key,const std::string &defVal) const
{
    const auto it = valueMap.find(key);
    if (it == valueMap.end())
        return defVal;
    return stringVals[it->second.entry];
}

DictionaryRef MutableDictionaryC::getDict(const std::string &name) const
{
    const auto it = stringMap.find(name);
    if (it == stringMap.end())
        return DictionaryRef();
    return getDict(it->second);
}

DictionaryRef MutableDictionaryC::getDict(unsigned int key) const
{
    const auto it = valueMap.find(key);
    if (it != valueMap.end()) {
        const auto &val = it->second;
        if (val.type == DictTypeDictionary)
            return dictVals[val.entry];
    }
    return DictionaryRef();
}

DictionaryEntryRef MutableDictionaryC::getEntry(const std::string &name) const
{
    const auto it = stringMap.find(name);
    if (it == stringMap.end())
        return DictionaryEntryRef();
    return getEntry(it->second);
}

DictionaryEntryRef MutableDictionaryC::getEntry(unsigned int key) const
{
    const auto it = valueMap.find(key);
    if (it == valueMap.end())
        return DictionaryEntryRef();
    
    const auto &val = it->second;
    switch (val.type) {
        case DictTypeInt:
            return std::make_shared<DictionaryEntryCBasic>(intVals[val.entry]);
        case DictTypeIdentity:
        case DictTypeInt64:
            return std::make_shared<DictionaryEntryCBasic>(int64Vals[val.entry]);
        case DictTypeDouble:
            return std::make_shared<DictionaryEntryCBasic>(dVals[val.entry]);
        case DictTypeString:
            return std::make_shared<DictionaryEntryCString>(stringVals[val.entry]);
        case DictTypeDictionary:
            return std::make_shared<DictionaryEntryCDict>(dictVals[val.entry]);
        case DictTypeArray:
            return std::make_shared<DictionaryEntryCArray>(formArray(val.entry));
        case DictTypeObject:
        case DictTypeNone:
            return DictionaryEntryRef();
    }
}

std::vector<DictionaryEntryRef> MutableDictionaryC::getArray(const std::string &name) const
{
    const auto it = stringMap.find(name);
    if (it == stringMap.end())
        return std::vector<DictionaryEntryRef>();
    return getArray(it->second);
}

std::vector<DictionaryEntryRef> MutableDictionaryC::getArray(unsigned int key) const
{
    const auto it = valueMap.find(key);
    if (it == valueMap.end() || it->second.type != DictTypeArray)
        return std::vector<DictionaryEntryRef>();

    // TODO: There's probalby a one line way to do this
    std::vector<DictionaryEntryRef> rets;
    rets.reserve(arrayVals[it->second.entry].size());
    const auto foo = formArray(it->second.entry);
    for (auto &val : foo)
        rets.push_back(val);
    return rets;
}

std::vector<DictionaryEntryCRef> MutableDictionaryC::formArray(int idx) const
{
    const auto &arrVals = arrayVals[idx];

    std::vector<DictionaryEntryCRef> rets;
    rets.reserve(arrVals.size());

    for (const auto &arrEntry: arrVals) {
        switch (arrEntry.type) {
            case DictTypeInt:
                rets.push_back(std::make_shared<DictionaryEntryCBasic>(intVals[arrEntry.entry]));
                break;
            case DictTypeInt64:
            case DictTypeIdentity:
                rets.push_back(std::make_shared<DictionaryEntryCBasic>(int64Vals[arrEntry.entry]));
                break;
            case DictTypeDouble:
                rets.push_back(std::make_shared<DictionaryEntryCBasic>(dVals[arrEntry.entry]));
                break;
            case DictTypeString:
                rets.push_back(std::make_shared<DictionaryEntryCString>(stringVals[arrEntry.entry]));
                break;
            case DictTypeDictionary:
                rets.push_back(std::make_shared<DictionaryEntryCDict>(dictVals[arrEntry.entry]));
                break;
            case DictTypeArray:
                rets.push_back(std::make_shared<DictionaryEntryCArray>(formArray(arrEntry.entry)));
                break;
            case DictTypeObject:
            case DictTypeNone:
                break;
        }
    }
    
    return rets;
}

std::vector<std::string> MutableDictionaryC::getKeys() const
{
    std::vector<std::string> keys;
    keys.reserve(valueMap.size());
    for (const auto &value : valueMap)
        keys.push_back(stringVals[value.first]);
    
    return keys;
}

int MutableDictionaryC::getKeyID(const std::string &name)
{
    const auto &it = stringMap.find(name);
    if (it == stringMap.end())
        return -1;
    return it->second;
}

void MutableDictionaryC::setInt(const std::string &name,int val)
{
    setInt(addKeyID(name),val);
}
void MutableDictionaryC::setInt(unsigned int key,int val)
{
    set(key, val, DictTypeInt, intVals);
}

void MutableDictionaryC::setIdentifiable(const std::string &name,SimpleIdentity val)
{
    setIdentifiable(addKeyID(name),val);
}
void MutableDictionaryC::setIdentifiable(unsigned int key,SimpleIdentity val)
{
    set(key, val, DictTypeIdentity, DictTypeInt64, int64Vals);
}

void MutableDictionaryC::setDouble(const std::string &name,double val)
{
    setDouble(addKeyID(name),val);
}
void MutableDictionaryC::setDouble(unsigned int key,double val)
{
    set(key, val, DictTypeDouble, dVals);
}

void MutableDictionaryC::setString(const std::string &name,const std::string &val)
{
    setString(addKeyID(name),val);
}
void MutableDictionaryC::setString(unsigned int key,const std::string &val)
{
    const auto &it = valueMap.find(key);
    // Can't reuse string entries because we share them
    if (it != valueMap.end())
        valueMap.erase(it);
    
    // Make a new one (psst, it's the same as the keys)
    auto stringID = addString(val);
    Value entry(DictTypeString,stringID);
    valueMap[key] = entry;
}

void MutableDictionaryC::setDict(const std::string &name,MutableDictionaryCRef dict)
{
    setDict(addKeyID(name),dict);
}
void MutableDictionaryC::setDict(unsigned int key,MutableDictionaryCRef dict)
{
    set(key, std::ref(dict), DictTypeDictionary, dictVals);
}

std::vector<MutableDictionaryC::Value> MutableDictionaryC::setupArray(const std::vector<DictionaryEntryCRef> &entries)
{
    std::vector<Value> out;
    out.reserve(entries.size());

    for (auto &entry: entries) {
        if (entry) {
            switch (entry->getType()) {
                case DictTypeInt:
                    out.push_back(Value(DictTypeInt,intVals.size()));
                    intVals.push_back(entry->getInt());
                    break;
                case DictTypeIdentity:
                case DictTypeInt64:
                    out.push_back(Value(DictTypeInt64,int64Vals.size()));
                    int64Vals.push_back(entry->getInt());
                    break;
                case DictTypeDouble:
                    out.push_back(Value(DictTypeDouble,dVals.size()));
                    dVals.push_back(entry->getInt());
                    break;
                case DictTypeString:
                    out.push_back(Value(DictTypeString,addString(entry->getString())));
                    break;
                case DictTypeDictionary:
                {
                    auto theDict = std::dynamic_pointer_cast<MutableDictionaryC>(entry->getDict());
                    if (theDict) {
                        out.push_back(Value(DictTypeDictionary,dictVals.size()));
                        dictVals.push_back(theDict);
                    }
                }
                    break;
                case DictTypeArray:
                {
                    auto theArray = std::dynamic_pointer_cast<DictionaryEntryCArray>(entry);
                    if (theArray && !theArray->vals.empty()) {
                        out.push_back(Value(DictTypeArray,arrayVals.size()));
                        auto locArr = setupArray(theArray->getArrayC());
                        arrayVals.push_back(locArr);
                    }
                }
                    break;
                case DictTypeNone:
                case DictTypeObject:
                    break;
            }
        }
    }
    
    return out;
}

void MutableDictionaryC::setArray(const std::string &name,std::vector<DictionaryEntryRef> &entries)
{
    setArray(addKeyID(name),entries);
}
void MutableDictionaryC::setArray(unsigned int key,std::vector<DictionaryEntryRef> &entries)
{
    const auto &it = valueMap.find(key);
    // Clear out the field.  Just easier
    if (it != valueMap.end())
        valueMap.erase(it);
    
    // TODO: Can we cast this once?
    std::vector<DictionaryEntryCRef> theEntries;
    for (auto &entry : entries)
        if (auto entryC = std::dynamic_pointer_cast<DictionaryEntryC>(entry))
            theEntries.push_back(entryC);
    auto newArray = setupArray(theEntries);

    Value entry(DictTypeArray,arrayVals.size());
    arrayVals.push_back(newArray);
    valueMap[key] = entry;
}

void MutableDictionaryC::setArray(const std::string &name,std::vector<DictionaryRef> &entries)
{
    setArray(addKeyID(name),entries);
}
void MutableDictionaryC::setArray(unsigned int key,std::vector<DictionaryRef> &entries)
{
    std::vector<DictionaryEntryRef> theEntries;
    for (auto &entry : entries) {
        if (auto theEntry = std::dynamic_pointer_cast<DictionaryEntryCDict>(entry))
            theEntries.push_back(theEntry);
    }

    setArray(key,theEntries);
}

void MutableDictionaryC::addEntries(const Dictionary *inOther)
{
    if (const auto other = dynamic_cast<const MutableDictionaryC *>(inOther))
        addEntries(other);
}

void MutableDictionaryC::addEntries(const MutableDictionaryC *other)
{
    // Map from theirs to our strings
    std::vector<unsigned int> stringRemap;
    stringRemap.reserve(other->stringVals.size());
    for (const auto &entry : other->stringVals) {
        auto newStringID = addString(entry);
        stringRemap.push_back(newStringID);
    }

    // Some data types we can just append
    const auto intStart = intVals.size();
    intVals.reserve(intVals.size() + other->intVals.size());
    intVals.insert(intVals.end(), other->intVals.begin(), other->intVals.end());
    
    const auto int64Start = int64Vals.size();
    int64Vals.reserve(int64Vals.size() + other->int64Vals.size());
    int64Vals.insert(int64Vals.end(), other->int64Vals.begin(), other->int64Vals.end());
    
    const auto dStart = dVals.size();
    dVals.reserve(dVals.size() + other->dVals.size());
    dVals.insert(dVals.end(), other->dVals.begin(), other->dVals.end());
    
    const auto dictStart = dictVals.size();
    dictVals.reserve(dictVals.size() + other->dictVals.size());
    dictVals.insert(dictVals.end(), other->dictVals.begin(), other->dictVals.end());
    
    // Array values need to be modified individually
    const auto arrayStart = arrayVals.size();
    for (const auto &arr: other->arrayVals) {
        std::vector<Value> outArr;
        for (const auto &arrEntry: arr) {
            switch (arrEntry.type) {
                case DictTypeString:
                    outArr.push_back(Value(DictTypeString,stringRemap[arrEntry.entry]));
                    break;
                case DictTypeInt:
                    outArr.push_back(Value(DictTypeInt,arrEntry.entry+intStart));
                    break;
                case DictTypeInt64:
                case DictTypeIdentity:
                    outArr.push_back(Value(DictTypeInt64,arrEntry.entry+int64Start));
                    break;
                case DictTypeDouble:
                    outArr.push_back(Value(DictTypeDouble,arrEntry.entry+dStart));
                    break;
                case DictTypeDictionary:
                    outArr.push_back(Value(DictTypeDictionary,arrEntry.entry+dictStart));
                    break;
                case DictTypeArray:
                    outArr.push_back(Value(DictTypeArray,arrEntry.entry+arrayStart));
                    break;
                case DictTypeObject:
                case DictTypeNone:
                    break;
            }
        }
        arrayVals.push_back(outArr);
    }
    
    // Now we map the values into their new locations
    for (const auto &value: other->valueMap) {
        unsigned int newKey = stringRemap[value.first];
        Value val = value.second;
        switch (val.type) {
            case DictTypeString:
                val.entry = stringRemap[val.entry];
                break;
            case DictTypeInt:
                val.entry = val.entry + intStart;
                break;
            case DictTypeInt64:
            case DictTypeIdentity:
                val.entry = val.entry + int64Start;
                break;
            case DictTypeDouble:
                val.entry = val.entry + dStart;
                break;
            case DictTypeDictionary:
                val.entry = val.entry + dictStart;
                break;
            case DictTypeArray:
                val.entry = val.entry + arrayStart;
                break;
            case DictTypeObject:
            case DictTypeNone:
                break;
        }
        valueMap[newKey] = val;
    }
}

unsigned int MutableDictionaryC::addString(const std::string &name)
{
    const auto res = stringMap.insert(std::make_pair(name,stringVals.size()));
    if (res.second) {
        // new key inserted
        stringVals.push_back(name);
    }
    return res.first->second;
}

int DictionaryEntryCBasic::getInt() const
{
    switch (type) {
        case DictTypeInt:
            return val.iVal;
        case DictTypeIdentity:
        case DictTypeInt64:
            return val.i64Val;
        case DictTypeDouble:
            return (int)val.dVal;
        default:
            return 0;
    }
}

SimpleIdentity DictionaryEntryCBasic::getIdentity() const
{
    switch (type) {
        case DictTypeInt:
            return val.iVal;
        case DictTypeIdentity:
        case DictTypeInt64:
            return val.i64Val;
        case DictTypeDouble:
            return val.dVal;
        default:
            return 0;
    }
}

int64_t DictionaryEntryCBasic::getInt64() const
{
    switch (type) {
        case DictTypeInt:
            return val.iVal;
        case DictTypeIdentity:
        case DictTypeInt64:
            return val.i64Val;
        case DictTypeDouble:
            return val.dVal;
        default:
            return 0;
    }
}

bool DictionaryEntryCBasic::getBool() const
{
    switch (type) {
        case DictTypeInt:
            return val.iVal != 0;
        case DictTypeIdentity:
        case DictTypeInt64:
            return val.i64Val != 0;
        case DictTypeDouble:
            return val.dVal != 0.0;
        default:
            return 0;
    }
}

RGBAColor DictionaryEntryCBasic::getColor() const
{
    RGBAColor ret;
    ret.b = val.iVal & 0xFF;
    ret.g = (val.iVal >> 8) & 0xFF;
    ret.r = (val.iVal >> 16) & 0xFF;
    ret.a = (val.iVal >> 24) & 0xFF;
    
    return ret;
}

double DictionaryEntryCBasic::getDouble() const
{
    switch (type) {
        case DictTypeInt:
            return val.iVal;
        case DictTypeIdentity:
        case DictTypeInt64:
            return val.i64Val;
        case DictTypeDouble:
            return val.dVal;
        default:
            return 0;
    }
}

bool DictionaryEntryCBasic::isEqual(DictionaryEntryRef other) const
{
    const auto otherRef = std::dynamic_pointer_cast<DictionaryEntryCBasic>(other);
    if (!otherRef)
        return false;
    
    switch (type) {
        case DictTypeInt:
            return val.iVal == otherRef->getInt();
        case DictTypeInt64:
        case DictTypeIdentity:
            return val.i64Val == otherRef->getInt64();
        case DictTypeDouble:
            return val.dVal == otherRef->getDouble();
        default:
            return false;
    }
}

RGBAColor DictionaryEntryCString::getColor() const
{
    // We're looking for a #RRGGBBAA
    if (str.length() < 1 || str[0] != '#')
        return RGBAColor::white();

    const int iVal = atoi(&str.c_str()[1]);
    RGBAColor ret;
    ret.b = iVal & 0xFF;
    ret.g = (iVal >> 8) & 0xFF;
    ret.r = (iVal >> 16) & 0xFF;
    ret.a = (iVal >> 24) & 0xFF;

    return ret;
}

bool DictionaryEntryCString::isEqual(DictionaryEntryRef other) const
{
    const auto otherRef = std::dynamic_pointer_cast<DictionaryEntryCString>(other);
    return otherRef && (str == otherRef->str);
}

bool DictionaryEntryCDict::isEqual(DictionaryEntryRef other) const
{
    // TODO: Actually make this work
    return false;
}

DictionaryEntryCArray::DictionaryEntryCArray(const std::vector<DictionaryEntryRef> &inVals) : DictionaryEntryC(DictTypeArray)
{
    vals.reserve(inVals.size());
    for (const auto &val : inVals) {
        if (const auto p = std::dynamic_pointer_cast<DictionaryEntryC>(val)) {
            vals.push_back(p);
        }
    }
}

std::vector<DictionaryEntryRef> DictionaryEntryCArray::getArray() const
{
    std::vector<DictionaryEntryRef> rets;
    rets.reserve(vals.size());
    for (const auto &val : vals) {
        if (const auto p = std::dynamic_pointer_cast<DictionaryEntry>(val)) {
            rets.push_back(p);
        }
    }
    
    return rets;
}

std::vector<DictionaryEntryCRef> DictionaryEntryCArray::getArrayC() const
{
    return vals;
}

bool DictionaryEntryCArray::isEqual(DictionaryEntryRef other) const
{
    // TODO: Make this work
    return false;
}


}
