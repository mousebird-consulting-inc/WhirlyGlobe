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
        
int MutableDictionaryC::StringValue::asInt()
{
    std::stringstream convert(val);
    int res;
    if (!(convert >> res))
        res = 0;
    
    return res;
}

SimpleIdentity MutableDictionaryC::StringValue::asIdentity()
{
    std::stringstream convert(val);
    SimpleIdentity res;
    if (!(convert >> res))
        res = 0;
    
    return res;
}

double MutableDictionaryC::StringValue::asDouble()
{
    std::stringstream convert(val);
    double res;
    if (!(convert >> res))
        res = 0;
    
    return res;
}
    
void MutableDictionaryC::IntValue::asString(std::string &retStr)
{
    std::ostringstream stream;
    stream << val;
    retStr = stream.str();
}
    
void MutableDictionaryC::DoubleValue::asString(std::string &retStr)
{
    std::ostringstream stream;
    stream << val;
    retStr = stream.str();
}
    
void MutableDictionaryC::IdentityValue::asString(std::string &retStr)
{
    std::ostringstream stream;
    stream << val;
    retStr = stream.str();
}

DictionaryRef MutableDictionaryC::DictionaryValue::asDict()
{
    return val;
}

MutableDictionaryC::ArrayValue::ArrayValue(const std::vector<DictionaryEntryRef> &entries)
{
    for (auto entry: entries) {
        ValueRef valRef = makeValueRef(std::dynamic_pointer_cast<DictionaryEntryC>(entry));
        if (valRef)
            val.push_back(valRef);
    }
}

MutableDictionaryC::ArrayValue::ArrayValue(const std::vector<DictionaryRef> &entries)
{
    for (auto entry: entries) {
        ValueRef valRef(new DictionaryValue(std::dynamic_pointer_cast<MutableDictionaryC>(entry)));
        if (valRef)
           val.push_back(valRef);
    }
}

    
MutableDictionaryRef MutableDictionaryC::copy()
{
    return std::make_shared<MutableDictionaryC>(*this);
}
    
MutableDictionaryC::MutableDictionaryC()
{
}
    
MutableDictionaryC::MutableDictionaryC(const MutableDictionaryC &that)
{
    for (FieldMap::const_iterator it = that.fields.begin();it != that.fields.end();++it)
        fields[it->first] = it->second->copy();
}
    
MutableDictionaryC::~MutableDictionaryC()
{
    clear();
}

bool MutableDictionaryC::parseJSON(const std::string jsonString)
{
    json_string json = jsonString;

    JSONNode topNode = libjson::parse(json);
    return parseJSONNode(topNode);
}

MutableDictionaryC::ValueRef MutableDictionaryC::parseJSONValue(JSONNode::iterator &nodeIt)
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
            MutableDictionaryCRef dict(new MutableDictionaryC());
            auto node = nodeIt->as_node();
            dict->parseJSONNode(node);
            return DictionaryValueRef(new DictionaryValue(dict));
        }
            break;
    }

    return ValueRef();
}

bool MutableDictionaryC::parseJSONNode(JSONNode &node)
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

void MutableDictionaryC::clear()
{
    fields.clear();
}
    
MutableDictionaryC &MutableDictionaryC::operator = (const MutableDictionaryC &that)
{
    clear();
    for (FieldMap::const_iterator it = that.fields.begin();it != that.fields.end();++it)
        fields[it->first] = it->second->copy();
    
    return *this;
}
    
MutableDictionaryC::MutableDictionaryC(RawData *rawData)
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
    
void MutableDictionaryC::asRawData(MutableRawData *rawData)
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
    
int MutableDictionaryC::numFields() const
{
    return (int)fields.size();
}
    
bool MutableDictionaryC::hasField(const std::string &name) const
{
    FieldMap::const_iterator it = fields.find(name);
        return (it != fields.end());
}
    
DictionaryType MutableDictionaryC::getType(const std::string &name) const
{
    FieldMap::const_iterator it = fields.find(name);
    if (it == fields.end())
        return DictTypeNone;
    
    return it->second->type();
}
    
void MutableDictionaryC::removeField(const std::string &name)
{
    FieldMap::iterator it = fields.find(name);
    if (it != fields.end())
        fields.erase(it);
}
    
int MutableDictionaryC::getInt(const std::string &name,int defVal) const
{
    FieldMap::const_iterator it = fields.find(name);
    if (it == fields.end())
        return defVal;
    
    return it->second->asInt();
}
    
SimpleIdentity MutableDictionaryC::getIdentity(const std::string &name) const
{
    FieldMap::const_iterator it = fields.find(name);
    if (it == fields.end())
        return EmptyIdentity;
    
    return it->second->asIdentity();
}
    
bool MutableDictionaryC::getBool(const std::string &name,bool defVal) const
{
    FieldMap::const_iterator it = fields.find(name);
    if (it == fields.end())
        return defVal;
    
    return (bool)it->second->asInt();
}

RGBAColor MutableDictionaryC::getColor(const std::string &name,const RGBAColor &defVal) const
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
    
double MutableDictionaryC::getDouble(const std::string &name,double defVal) const
{
    FieldMap::const_iterator it = fields.find(name);
    if (it == fields.end())
        return defVal;
    
    return it->second->asDouble();
}
    
std::string MutableDictionaryC::getString(const std::string &name) const
{
    FieldMap::const_iterator it = fields.find(name);
    if (it == fields.end())
        return "";
    
    std::string retStr;
    it->second->asString(retStr);
    return retStr;
}

std::string MutableDictionaryC::getString(const std::string &name,const std::string &defVal) const
{
    FieldMap::const_iterator it = fields.find(name);
    if (it == fields.end())
        return defVal;
    
    std::string retStr;
    it->second->asString(retStr);
    return retStr;
}
    
DelayedDeletableRef MutableDictionaryC::getObject(const std::string &name)
{
    FieldMap::const_iterator it = fields.find(name);
    if (it == fields.end())
        return DelayedDeletableRef();
    
    return DelayedDeletableRef(it->second->asObject());
}

DictionaryRef MutableDictionaryC::getDict(const std::string &name) const
{
    FieldMap::const_iterator it = fields.find(name);
    if (it == fields.end())
        return DictionaryRef();

    MutableDictionaryC::DictionaryValueRef dictVal = std::dynamic_pointer_cast<DictionaryValue>(it->second);
    if (dictVal)
        return dictVal->val;

    return DictionaryRef();
}

DictionaryEntryRef MutableDictionaryC::getEntry(const std::string &name) const
{
    FieldMap::const_iterator it = fields.find(name);
    if (it == fields.end())
        return DictionaryEntryRef();

    return std::make_shared<DictionaryEntryC>(it->second);
}

MutableDictionaryC::ValueRef MutableDictionaryC::getValueRef(const std::string &name) const
{
    FieldMap::const_iterator it = fields.find(name);
    if (it == fields.end())
        return ValueRef();

    return it->second;
}

std::vector<DictionaryEntryRef> MutableDictionaryC::getArray(const std::string &name) const
{
    FieldMap::const_iterator it = fields.find(name);
    if (it == fields.end())
        return std::vector<DictionaryEntryRef>();

    if (it->second->type() == DictTypeArray) {
        ArrayValueRef val = std::dynamic_pointer_cast<ArrayValue>(it->second);
        if (val) {
            std::vector<DictionaryEntryRef> ret;
            for (auto entry: val->val)
                ret.push_back(std::make_shared<DictionaryEntryC>(entry));
            return ret;
        }
    }

    return std::vector<DictionaryEntryRef>();
}

std::vector<std::string> MutableDictionaryC::getKeys() const
{
    std::vector<std::string> keys;
    for (auto it: fields)
        keys.push_back(it.first);

    return keys;
}

void MutableDictionaryC::setInt(const std::string &name,int val)
{
    removeField(name);
    
    IntValue *iVal = new IntValue();
    iVal->val = val;
    fields[name] = ValueRef(iVal);
}
    
void MutableDictionaryC::setIdentifiable(const std::string &name,SimpleIdentity val)
{
    removeField(name);
    
    IdentityValue *iVal = new IdentityValue();
    iVal->val = val;
    fields[name] = ValueRef(iVal);
}

void MutableDictionaryC::setDouble(const std::string &name,double val)
{
    removeField(name);
    
    DoubleValue *dVal = new DoubleValue();
    dVal->val = val;
    fields[name] = ValueRef(dVal);
}

void MutableDictionaryC::setString(const std::string &name,const std::string &val)
{
    removeField(name);
    
    StringValue *sVal = new StringValue();
    sVal->val = val;
    fields[name] = ValueRef(sVal);
}

void MutableDictionaryC::setDict(const std::string &name,MutableDictionaryCRef dict)
{
    removeField(name);

    DictionaryValue *dVal = new DictionaryValue();
    dVal->val = dict;
    fields[name] = ValueRef(dVal);
}

MutableDictionaryC::ValueRef MutableDictionaryC::makeValueRef(DictionaryEntryCRef entry)
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
            value = new DictionaryValue(std::dynamic_pointer_cast<MutableDictionaryC>(entry->getDict()));
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

void MutableDictionaryC::setEntry(const std::string &name,DictionaryEntryCRef entry)
{
    removeField(name);

    fields[name] = makeValueRef(entry);
}

void MutableDictionaryC::setArray(const std::string &name,std::vector<DictionaryEntryRef> &entries)
{
    removeField(name);

    ArrayValue *aVal = new ArrayValue(entries);
    fields[name] = ValueRef(aVal);
}

void MutableDictionaryC::setArray(const std::string &name,std::vector<DictionaryRef> &entries)
{
    removeField(name);

    ArrayValue *aVal = new ArrayValue(entries);
    fields[name] = ValueRef(aVal);
}
    
void MutableDictionaryC::setObject(const std::string &name, DelayedDeletableRef obj)
{
    removeField(name);
    
    ObjectValue *oVal = new ObjectValue();
    oVal->val = obj;
    fields[name] = ValueRef(oVal);
}
    
std::string MutableDictionaryC::toString() const
{
    std::string str;
    for (const auto &it : fields)
    {
        std::string valStr;
        it.second->asString(valStr);
        str += it.first + ":" + valStr + "\n";
    }
    
    return str;
}

void MutableDictionaryC::addEntries(const Dictionary *inOther)
{
    const MutableDictionaryC *other = dynamic_cast<const MutableDictionaryC *>(inOther);

    for (FieldMap::const_iterator it = other->fields.begin();it != other->fields.end();++it)
        fields[it->first] = it->second->copy();

}

DictionaryType DictionaryEntryC::getType() const
{
    return type;
}

int DictionaryEntryC::getInt() const
{
    return val->asInt();
}

SimpleIdentity DictionaryEntryC::getIdentity() const
{
    return val->asIdentity();
}

bool DictionaryEntryC::getBool() const
{
    return val->asInt() != 0;
}

RGBAColor DictionaryEntryC::getColor() const
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

double DictionaryEntryC::getDouble() const
{
    return val->asDouble();
}

std::string DictionaryEntryC::getString() const
{
    std::string str;
    val->asString(str);

    return str;
}

DictionaryRef DictionaryEntryC::getDict() const
{
    return val->asDict();
}

std::vector<DictionaryEntryRef> DictionaryEntryC::getArray() const
{
    if (type != DictTypeArray)
        return std::vector<DictionaryEntryRef>();

    MutableDictionaryC::ArrayValueRef theVal = std::dynamic_pointer_cast<MutableDictionaryC::ArrayValue>(val);
    if (theVal) {
        std::vector<DictionaryEntryRef> ret;
        for (auto entry: theVal->val)
            ret.push_back(DictionaryEntryRef(new DictionaryEntryC(entry)));
        return ret;
    }

    return std::vector<DictionaryEntryRef>();
}

bool DictionaryEntryC::isEqual(DictionaryEntryRef inOther) const
{
    DictionaryEntryCRef other = std::dynamic_pointer_cast<DictionaryEntryC>(inOther);
    if (!other)
        return false;

    bool compat = type == other->getType();
    if (!compat) {
        if ((type == DictTypeInt || type == DictTypeDouble) &&
            (other->getType() == DictTypeInt || other->getType() == DictTypeDouble))
            compat = true;
    }
    if (!compat)
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

void DictionaryEntryC::setDouble(double inVal)
{
    type = DictTypeDouble;
    val = std::make_shared<MutableDictionaryC::DoubleValue>(inVal);
}

void DictionaryEntryC::setString(const std::string &inVal)
{
    type = DictTypeString;
    val = std::make_shared<MutableDictionaryC::StringValue>(inVal);
}

void DictionaryEntryC::setIdentity(SimpleIdentity inVal)
{
    type = DictTypeIdentity;
    val = std::make_shared<MutableDictionaryC::IdentityValue>(inVal);
}

void DictionaryEntryC::setArray(const std::vector<DictionaryEntryRef> &inArr)
{
    type = DictTypeArray;
    val = std::make_shared<MutableDictionaryC::ArrayValue>(inArr);
}

void DictionaryEntryC::setDictionary(MutableDictionaryCRef inVal)
{
    type = DictTypeDictionary;
    val = std::make_shared<MutableDictionaryC::DictionaryValue>(inVal);
}

}
