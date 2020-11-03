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
    
MutableDictionaryRef MutableDictionaryC::copy()
{
    return std::make_shared<MutableDictionaryC>(*this);
}
    
MutableDictionaryC::MutableDictionaryC()
{
}
    
MutableDictionaryC::MutableDictionaryC(const MutableDictionaryC &that)
{
    *this = that;
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
    
int MutableDictionaryC::numFields() const
{
    return valueMap.size();
}
    
bool MutableDictionaryC::hasField(const std::string &name) const
{
    const auto it = stringMap.find(name);
    if (it == stringMap.end())
        return false;
    return hasField(it->second);
}

bool MutableDictionaryC::hasField(unsigned int key) const
{
    const auto it = valueMap.find(key);
    if (it == valueMap.end())
        return false;
    return true;
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
    auto it = valueMap.find(key);
    if (it == valueMap.end())
        return;
    // TODO: WE're "leaking" (not actually) leaking space in the data arrays
    valueMap.erase(key);
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
            break;
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
            break;
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
            break;
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
            break;
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
            break;
        // No idea what this means
        case DictTypeDouble:
        default:
            return defVal;
            break;
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
            break;
        // TODO: Maybe parse the string
        default:
            return 0;
    }
}

std::string MutableDictionaryC::getString(const std::string &name) const
{
    const auto it = stringMap.find(name);
    if (it == stringMap.end())
        return "";
    return getString(it->second);
}

std::string MutableDictionaryC::getString(unsigned int key) const
{
    const auto it = valueMap.find(key);
    if (it == valueMap.end())
        return "";
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
            return "";
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
    if (it == valueMap.end())
        return DictionaryRef();
    
    const auto &val = it->second;
    if (val.type == DictTypeDictionary)
        return dictVals[val.entry];
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
            break;
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
    auto foo = formArray(it->second.entry);
    for (auto &val : foo)
        rets.push_back(val);
    return rets;
}

std::vector<DictionaryEntryCRef> MutableDictionaryC::formArray(int idx) const
{
    std::vector<DictionaryEntryCRef> rets;

    const auto &arrVals = arrayVals[idx];
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
    const auto &it = valueMap.find(key);
    // Reuse the entry if it's there
    if (it != valueMap.end()) {
        if (it->second.type != DictTypeInt) {
            removeField(key);
        } else {
            intVals[it->second.entry] = val;
            return;
        }
    }
    
    // Make a new one
    Value entry(DictTypeInt,intVals.size());
    intVals.push_back(val);
    valueMap[key] = entry;
}

void MutableDictionaryC::setIdentifiable(const std::string &name,SimpleIdentity val)
{
    setIdentifiable(addKeyID(name),val);
}
void MutableDictionaryC::setIdentifiable(unsigned int key,SimpleIdentity val)
{
    const auto &it = valueMap.find(key);
    // Reuse the entry if it's there
    if (it != valueMap.end()) {
        if (it->second.type != DictTypeIdentity && it->second.type != DictTypeInt64) {
            removeField(key);
        } else {
            int64Vals[it->second.entry] = val;
            return;
        }
    }
    
    // Make a new one
    Value entry(DictTypeIdentity,int64Vals.size());
    int64Vals.push_back(val);
    valueMap[key] = entry;
}

void MutableDictionaryC::setDouble(const std::string &name,double val)
{
    setDouble(addKeyID(name),val);
}
void MutableDictionaryC::setDouble(unsigned int key,double val)
{
    const auto &it = valueMap.find(key);
    // Reuse the entry if it's there
    if (it != valueMap.end()) {
        if (it->second.type != DictTypeDouble) {
            removeField(key);
        } else {
            dVals[it->second.entry] = val;
            return;
        }
    }
    
    // Make a new one
    Value entry(DictTypeDouble,dVals.size());
    dVals.push_back(val);
    valueMap[key] = entry;
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
        removeField(key);
    
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
    const auto &it = valueMap.find(key);
    // Reuse the entry if it's there
    if (it != valueMap.end()) {
        if (it->second.type != DictTypeDictionary) {
            removeField(key);
        } else {
            dictVals[it->second.entry] = dict;
            return;
        }
    }
    
    // Make a new one
    Value entry(DictTypeDictionary,dictVals.size());
    dictVals.push_back(dict);
    valueMap[key] = entry;
}

std::vector<MutableDictionaryC::Value> MutableDictionaryC::setupArray(const std::vector<DictionaryEntryCRef> &entries)
{
    std::vector<Value> out;

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
        removeField(key);
    
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
    const MutableDictionaryC *other = dynamic_cast<const MutableDictionaryC *>(inOther);
    if (!other)
        return;

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
    auto intStart = intVals.size();  intVals.insert(intVals.end(), other->intVals.begin(), other->intVals.end());
    auto int64Start = int64Vals.size(); int64Vals.insert(int64Vals.end(), other->int64Vals.begin(), other->int64Vals.end());
    auto dStart = dVals.size();  dVals.insert(dVals.end(), other->dVals.begin(), other->dVals.end());
    auto dictStart = dictVals.size();  dictVals.insert(dictVals.end(), other->dictVals.begin(), other->dictVals.end());
    
    // Array values need to be modified individually
    auto arrayStart = arrayVals.size();
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

unsigned int MutableDictionaryC::addKeyID(const std::string &name)
{
    return addString(name);
}

unsigned int MutableDictionaryC::addString(const std::string &name)
{
    const auto &it = stringMap.find(name);
    if (it != stringMap.end())
        return it->second;
    
    auto idx = stringVals.size();
    stringVals.push_back(name);
    stringMap[name] = idx;
    
    return idx;
}

DictionaryEntryCBasic::DictionaryEntryCBasic(int iVal) : DictionaryEntryC(DictTypeInt)
{
    val.iVal = iVal;
}

DictionaryEntryCBasic::DictionaryEntryCBasic(double dVal) : DictionaryEntryC(DictTypeDouble)
{
    val.dVal = dVal;
}

DictionaryEntryCBasic::DictionaryEntryCBasic(int64_t iVal) : DictionaryEntryC(DictTypeInt64)
{
    val.i64Val = iVal;
}

int DictionaryEntryCBasic::getInt() const
{
    switch (type) {
        case DictTypeInt:
            return val.iVal;
            break;
        case DictTypeIdentity:
        case DictTypeInt64:
            return val.i64Val;
            break;
        case DictTypeDouble:
            return (int)val.dVal;
            break;
        default:
            return 0;
    }
}

SimpleIdentity DictionaryEntryCBasic::getIdentity() const
{
    switch (type) {
        case DictTypeInt:
            return val.iVal;
            break;
        case DictTypeIdentity:
        case DictTypeInt64:
            return val.i64Val;
            break;
        case DictTypeDouble:
            return val.dVal;
            break;
        default:
            return 0;
    }
}

int64_t DictionaryEntryCBasic::getInt64() const
{
    switch (type) {
        case DictTypeInt:
            return val.iVal;
            break;
        case DictTypeIdentity:
        case DictTypeInt64:
            return val.i64Val;
            break;
        case DictTypeDouble:
            return val.dVal;
            break;
        default:
            return 0;
    }
}

bool DictionaryEntryCBasic::getBool() const
{
    switch (type) {
        case DictTypeInt:
            return val.iVal != 0;
            break;
        case DictTypeIdentity:
        case DictTypeInt64:
            return val.i64Val != 0;
            break;
        case DictTypeDouble:
            return val.dVal != 0.0;
            break;
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
            break;
        case DictTypeIdentity:
        case DictTypeInt64:
            return val.i64Val;
            break;
        case DictTypeDouble:
            return val.dVal;
            break;
        default:
            return 0;
    }
}

bool DictionaryEntryCBasic::isEqual(DictionaryEntryRef other) const
{
    auto otherRef = std::dynamic_pointer_cast<DictionaryEntryCBasic>(other);
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

DictionaryEntryCString::DictionaryEntryCString(const std::string &str) : DictionaryEntryC(DictTypeString), str(str)
{
}

std::string DictionaryEntryCString::getString() const
{
    return str;
}

RGBAColor DictionaryEntryCString::getColor() const
{
    // We're looking for a #RRGGBBAA
    if (str.length() < 1 || str[0] != '#')
        return RGBAColor::white();

    int iVal = atoi(&str.c_str()[1]);
    RGBAColor ret;
    ret.b = iVal & 0xFF;
    ret.g = (iVal >> 8) & 0xFF;
    ret.r = (iVal >> 16) & 0xFF;
    ret.a = (iVal >> 24) & 0xFF;

    return ret;
}

bool DictionaryEntryCString::isEqual(DictionaryEntryRef other) const
{
    auto otherRef = std::dynamic_pointer_cast<DictionaryEntryCString>(other);
    if (!otherRef)
        return false;

    return str == otherRef->str;
}

DictionaryEntryCDict::DictionaryEntryCDict(const MutableDictionaryCRef &dict) : DictionaryEntryC(DictTypeDictionary), dict(dict)
{
}

DictionaryRef DictionaryEntryCDict::getDict() const
{
    return dict;
}

bool DictionaryEntryCDict::isEqual(DictionaryEntryRef other) const
{
    // TODO: Actually make this work
    return false;
}

DictionaryEntryCArray::DictionaryEntryCArray(const std::vector<DictionaryEntryCRef> &vals) : DictionaryEntryC(DictTypeArray), vals(vals)
{
}

DictionaryEntryCArray::DictionaryEntryCArray(const std::vector<DictionaryEntryRef> &inVals) : DictionaryEntryC(DictTypeArray)
{
    for (const auto &val : inVals)
        vals.push_back(std::dynamic_pointer_cast<DictionaryEntryC>(val));
}

std::vector<DictionaryEntryRef> DictionaryEntryCArray::getArray() const
{
    std::vector<DictionaryEntryRef> rets;
    for (const auto &val : vals)
        rets.push_back(std::dynamic_pointer_cast<DictionaryEntry>(val));
    
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
