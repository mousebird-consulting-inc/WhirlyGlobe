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
#import "Dictionary_Android.h"

namespace WhirlyKit
{
    
MutableDictionaryRef MutableDictionaryMake()
{
    return MutableDictionaryRef(new MutableDictionary_Android());
}
    
int MutableDictionary_Android::StringValue::asInt()
{
    std::stringstream convert(val);
    int res;
    if (!(convert >> res))
        res = 0;
    
    return res;
}

SimpleIdentity MutableDictionary_Android::StringValue::asIdentity()
{
    std::stringstream convert(val);
    SimpleIdentity res;
    if (!(convert >> res))
        res = 0;
    
    return res;
}

double MutableDictionary_Android::StringValue::asDouble()
{
    std::stringstream convert(val);
    double res;
    if (!(convert >> res))
        res = 0;
    
    return res;
}
    
void MutableDictionary_Android::IntValue::asString(std::string &retStr)
{
    std::ostringstream stream;
    stream << val;
    retStr = stream.str();
}
    
void MutableDictionary_Android::DoubleValue::asString(std::string &retStr)
{
    std::ostringstream stream;
    stream << val;
    retStr = stream.str();
}
    
void MutableDictionary_Android::IdentityValue::asString(std::string &retStr)
{
    std::ostringstream stream;
    stream << val;
    retStr = stream.str();
}

DictionaryRef MutableDictionary_Android::DictionaryValue::asDict()
{
    return val;
}

MutableDictionary_Android::ArrayValue::ArrayValue(std::vector<DictionaryEntryRef> &entries)
{
    for (auto entry: entries) {
        ValueRef valRef = makeValueRef(std::dynamic_pointer_cast<DictionaryEntry_Android>(entry));
        if (valRef)
            val.push_back(valRef);
    }
}

MutableDictionary_Android::ArrayValue::ArrayValue(std::vector<DictionaryRef> &entries)
{
    for (auto entry: entries) {
        ValueRef valRef(new DictionaryValue(std::dynamic_pointer_cast<MutableDictionary_Android>(entry)));
        if (valRef)
           val.push_back(valRef);
    }
}

    
MutableDictionaryRef MutableDictionary_Android::copy()
{
    return MutableDictionaryRef(new MutableDictionary_Android(*this));
}
    
MutableDictionary_Android::MutableDictionary_Android()
{
}
    
MutableDictionary_Android::MutableDictionary_Android(const MutableDictionary_Android &that)
{
    for (FieldMap::const_iterator it = that.fields.begin();it != that.fields.end();++it)
        fields[it->first] = it->second->copy();
}
    
MutableDictionary_Android::~MutableDictionary_Android()
{
    clear();
}

bool MutableDictionary_Android::parseJSON(const std::string jsonString)
{
    json_string json = jsonString;

    JSONNode topNode = libjson::parse(json);
    return parseJSONNode(topNode);
}

MutableDictionary_Android::ValueRef MutableDictionary_Android::parseJSONValue(JSONNode::iterator &nodeIt)
{
    switch (nodeIt->type()) {
        case JSON_NULL:
            break;
        case JSON_STRING:
            return ValueRef(new StringValue(nodeIt->as_string()));
            break;
        case JSON_NUMBER:
            return ValueRef(new DoubleValue(nodeIt->as_float()));
            break;
        case JSON_BOOL:
            return ValueRef(new IntValue(nodeIt->as_bool()));
            break;
        case JSON_ARRAY:
        {
            auto nodes = nodeIt->as_array();
            std::vector<ValueRef> values;
            for (JSONNode::iterator arrNodeIt = nodes.begin(); arrNodeIt != nodes.end(); ++arrNodeIt) {
                values.push_back(parseJSONValue(arrNodeIt));
            }
            return ArrayValueRef(new ArrayValue(values));
        }
            break;
        case JSON_NODE:
        {
            MutableDictionary_AndroidRef dict(new MutableDictionary_Android());
            auto node = nodeIt->as_node();
            dict->parseJSONNode(node);
            return DictionaryValueRef(new DictionaryValue(dict));
        }
            break;
    }

    return ValueRef();
}

bool MutableDictionary_Android::parseJSONNode(JSONNode &node)
{
    for (JSONNode::iterator nodeIt = node.begin(); nodeIt != node.end(); ++nodeIt) {
        auto name = nodeIt->name();
        ValueRef val = parseJSONValue(nodeIt);
        if (name.empty() || !val)
            return false;
        fields[name] = val;
    }

    return true;
}

void MutableDictionary_Android::clear()
{
    fields.clear();
}
    
MutableDictionary_Android &MutableDictionary_Android::operator = (const MutableDictionary_Android &that)
{
    clear();
    for (FieldMap::const_iterator it = that.fields.begin();it != that.fields.end();++it)
        fields[it->first] = it->second->copy();
    
    return *this;
}
    
MutableDictionary_Android::MutableDictionary_Android(RawData *rawData)
{
    RawDataReader dataRead(rawData);
    while (!dataRead.done())
    {
        int type;
        if (!dataRead.getInt(type))
            return;
        std::string attrName;
        if (!dataRead.getString(attrName))
            return;
        switch (type)
        {
            case DictTypeString:
            {
                std::string sVal;
                if (!dataRead.getString(sVal))
                    return;
                setString(attrName, sVal);
            }
                break;
            case DictTypeInt:
            {
                int iVal;
                if (!dataRead.getInt(iVal))
                    return;
                setInt(attrName, iVal);
            }
                break;
            case DictTypeDouble:
            {
                double dVal;
                if (!dataRead.getDouble(dVal))
                    return;
                setDouble(attrName, dVal);
            }
                break;
            default:
                return;
        }
    }
}
    
void MutableDictionary_Android::asRawData(MutableRawData *rawData)
{
    for (FieldMap::iterator it = fields.begin(); it != fields.end(); ++it)
    {
        ValueRef val = it->second;
        if (val->type() == DictTypeObject)
            continue;
        rawData->addInt(val->type());
        rawData->addString(it->first);
        switch (val->type())
        {
            case DictTypeString:
            {
                std::string str;
                val->asString(str);
                rawData->addString(str);
            }
                break;
            case DictTypeInt:
                rawData->addInt(val->asInt());
                break;
            case DictTypeDouble:
                rawData->addDouble(val->asDouble());
                break;
            default:
                throw 1;
                break;
        }
    }
}
    
int MutableDictionary_Android::numFields() const
{
    return (int)fields.size();
}
    
bool MutableDictionary_Android::hasField(const std::string &name) const
{
    FieldMap::const_iterator it = fields.find(name);
        return (it != fields.end());
}
    
DictionaryType MutableDictionary_Android::getType(const std::string &name) const
{
    FieldMap::const_iterator it = fields.find(name);
    if (it == fields.end())
        return DictTypeNone;
    
    return it->second->type();
}
    
void MutableDictionary_Android::removeField(const std::string &name)
{
    FieldMap::iterator it = fields.find(name);
    if (it != fields.end())
        fields.erase(it);
}
    
int MutableDictionary_Android::getInt(const std::string &name,int defVal) const
{
    FieldMap::const_iterator it = fields.find(name);
    if (it == fields.end())
        return defVal;
    
    return it->second->asInt();
}
    
SimpleIdentity MutableDictionary_Android::getIdentity(const std::string &name) const
{
    FieldMap::const_iterator it = fields.find(name);
    if (it == fields.end())
        return EmptyIdentity;
    
    return it->second->asIdentity();
}
    
bool MutableDictionary_Android::getBool(const std::string &name,bool defVal) const
{
    FieldMap::const_iterator it = fields.find(name);
    if (it == fields.end())
        return defVal;
    
    return (bool)it->second->asInt();
}

RGBAColor MutableDictionary_Android::getColor(const std::string &name,const RGBAColor &defVal) const
{
    FieldMap::const_iterator it = fields.find(name);
    if (it == fields.end())
        return defVal;

    switch (it->second->type())
    {
        case DictTypeString:
        {
            std::string str;
            it->second->asString(str);
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
            int iVal = it->second->asInt();
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
    
double MutableDictionary_Android::getDouble(const std::string &name,double defVal) const
{
    FieldMap::const_iterator it = fields.find(name);
    if (it == fields.end())
        return defVal;
    
    return it->second->asDouble();
}
    
std::string MutableDictionary_Android::getString(const std::string &name) const
{
    FieldMap::const_iterator it = fields.find(name);
    if (it == fields.end())
        return "";
    
    std::string retStr;
    it->second->asString(retStr);
    return retStr;
}

std::string MutableDictionary_Android::getString(const std::string &name,const std::string &defVal) const
{
    FieldMap::const_iterator it = fields.find(name);
    if (it == fields.end())
        return defVal;
    
    std::string retStr;
    it->second->asString(retStr);
    return retStr;
}
    
DelayedDeletableRef MutableDictionary_Android::getObject(const std::string &name)
{
    FieldMap::const_iterator it = fields.find(name);
    if (it == fields.end())
        return DelayedDeletableRef();
    
    return DelayedDeletableRef(it->second->asObject());
}

DictionaryRef MutableDictionary_Android::getDict(const std::string &name) const
{
    FieldMap::const_iterator it = fields.find(name);
    if (it == fields.end())
        return DictionaryRef();

    MutableDictionary_Android::DictionaryValueRef dictVal = std::dynamic_pointer_cast<DictionaryValue>(it->second);
    if (dictVal)
        return dictVal->val;

    return DictionaryRef();
}

DictionaryEntryRef MutableDictionary_Android::getEntry(const std::string &name) const
{
    FieldMap::const_iterator it = fields.find(name);
    if (it == fields.end())
        return DictionaryEntryRef();

    return DictionaryEntryRef(new DictionaryEntry_Android(it->second));
}

std::vector<DictionaryEntryRef> MutableDictionary_Android::getArray(const std::string &name) const
{
    FieldMap::const_iterator it = fields.find(name);
    if (it == fields.end())
        return std::vector<DictionaryEntryRef>();

    if (it->second->type() == DictTypeArray) {
        ArrayValueRef val = std::dynamic_pointer_cast<ArrayValue>(it->second);
        if (val) {
            std::vector<DictionaryEntryRef> ret;
            for (auto entry: val->val)
                ret.push_back(DictionaryEntryRef(new DictionaryEntry_Android(entry)));
            return ret;
        }
    }

    return std::vector<DictionaryEntryRef>();
}

std::vector<std::string> MutableDictionary_Android::getKeys() const
{
    std::vector<std::string> keys;
    for (auto it: fields)
        keys.push_back(it.first);

    return keys;
}

void MutableDictionary_Android::setInt(const std::string &name,int val)
{
    removeField(name);
    
    IntValue *iVal = new IntValue();
    iVal->val = val;
    fields[name] = ValueRef(iVal);
}
    
void MutableDictionary_Android::setIdentifiable(const std::string &name,SimpleIdentity val)
{
    removeField(name);
    
    IdentityValue *iVal = new IdentityValue();
    iVal->val = val;
    fields[name] = ValueRef(iVal);
}

void MutableDictionary_Android::setDouble(const std::string &name,double val)
{
    removeField(name);
    
    DoubleValue *dVal = new DoubleValue();
    dVal->val = val;
    fields[name] = ValueRef(dVal);
}

void MutableDictionary_Android::setString(const std::string &name,const std::string &val)
{
    removeField(name);
    
    StringValue *sVal = new StringValue();
    sVal->val = val;
    fields[name] = ValueRef(sVal);
}

void MutableDictionary_Android::setDict(const std::string &name,MutableDictionary_AndroidRef dict)
{
    removeField(name);

    DictionaryValue *dVal = new DictionaryValue();
    dVal->val = dict;
    fields[name] = ValueRef(dVal);
}

MutableDictionary_Android::ValueRef MutableDictionary_Android::makeValueRef(DictionaryEntry_AndroidRef entry)
{
    Value *value = NULL;

    switch(entry->getType()) {
        case DictTypeNone:
        default:
        case DictTypeArray: {
            std::vector<DictionaryEntryRef> entries;
            for (auto thisEntry: entry->getArray()) {
                entries.push_back(thisEntry);
            }
            value = new ArrayValue(entries);
        }
            break;
        case DictTypeDictionary:
            value = new DictionaryValue(std::dynamic_pointer_cast<MutableDictionary_Android>(entry->getDict()));
            break;
        case DictTypeIdentity:
            value = new IdentityValue(entry->getIdentity());
            break;
        case DictTypeInt:
            value = new IntValue(entry->getInt());
            break;
        case DictTypeDouble:
            value = new DoubleValue(entry->getDouble());
            break;
        case DictTypeString:
            value = new StringValue(entry->getString());
            break;
    }

    return ValueRef(value);
}

void MutableDictionary_Android::setEntry(const std::string &name,DictionaryEntry_AndroidRef entry)
{
    removeField(name);

    fields[name] = makeValueRef(entry);
}

void MutableDictionary_Android::setArray(const std::string &name,std::vector<DictionaryEntryRef> &entries)
{
    removeField(name);

    ArrayValue *aVal = new ArrayValue(entries);
    fields[name] = ValueRef(aVal);
}

void MutableDictionary_Android::setArray(const std::string &name,std::vector<DictionaryRef> &entries)
{
    removeField(name);

    ArrayValue *aVal = new ArrayValue(entries);
    fields[name] = ValueRef(aVal);
}
    
void MutableDictionary_Android::setObject(const std::string &name, DelayedDeletableRef obj)
{
    removeField(name);
    
    ObjectValue *oVal = new ObjectValue();
    oVal->val = obj;
    fields[name] = ValueRef(oVal);
}
    
std::string MutableDictionary_Android::toString() const
{
    std::string str;
    for (const auto it : fields)
    {
        std::string valStr;
        it.second->asString(valStr);
        str += it.first + ":" + valStr + "\n";
    }
    
    return str;
}

void MutableDictionary_Android::addEntries(const Dictionary *inOther)
{
    const MutableDictionary_Android *other = dynamic_cast<const MutableDictionary_Android *>(inOther);

    for (FieldMap::const_iterator it = other->fields.begin();it != other->fields.end();++it)
        fields[it->first] = it->second->copy();

}

DictionaryType DictionaryEntry_Android::getType() const
{
    return type;
}

int DictionaryEntry_Android::getInt() const
{
    return val->asInt();
}

SimpleIdentity DictionaryEntry_Android::getIdentity() const
{
    return val->asIdentity();
}

bool DictionaryEntry_Android::getBool() const
{
    return val->asInt() != 0;
}

RGBAColor DictionaryEntry_Android::getColor() const
{
    switch (type)
    {
        case DictTypeString:
        {
            std::string str;
            val->asString(str);
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
            break;
        case DictTypeInt:
        {
            int iVal = val->asInt();
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
            return RGBAColor::white();
            break;
    }
}

double DictionaryEntry_Android::getDouble() const
{
    return val->asDouble();
}

std::string DictionaryEntry_Android::getString() const
{
    std::string str;
    val->asString(str);

    return str;
}

DictionaryRef DictionaryEntry_Android::getDict() const
{
    return val->asDict();
}

std::vector<DictionaryEntryRef> DictionaryEntry_Android::getArray() const
{
    if (type != DictTypeArray)
        return std::vector<DictionaryEntryRef>();

    MutableDictionary_Android::ArrayValueRef theVal = std::dynamic_pointer_cast<MutableDictionary_Android::ArrayValue>(val);
    if (theVal) {
        std::vector<DictionaryEntryRef> ret;
        for (auto entry: theVal->val)
            ret.push_back(DictionaryEntryRef(new DictionaryEntry_Android(entry)));
        return ret;
    }

    return std::vector<DictionaryEntryRef>();
}

bool DictionaryEntry_Android::isEqual(DictionaryEntryRef inOther) const
{
    DictionaryEntry_AndroidRef other = std::dynamic_pointer_cast<DictionaryEntry_Android>(inOther);
    if (!other)
        return false;

    if (type != other->getType())
        return false;

    switch (type) {
        case DictTypeString:
            return getString() == other->getString();
            break;
        case DictTypeInt:
            return val->asInt() == other->getInt();
            break;
        case DictTypeIdentity:
            return val->asIdentity() == other->getIdentity();
            break;
        case DictTypeDouble:
            return val->asDouble() == other->getDouble();
            break;
        case DictTypeDictionary:
            return false;
            break;
        case DictTypeNone:
        case DictTypeObject:
        case DictTypeArray:
            return false;
            break;
    }
}

}
