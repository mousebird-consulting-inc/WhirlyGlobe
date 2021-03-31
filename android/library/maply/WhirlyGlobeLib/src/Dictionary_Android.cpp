/*  Dictionary.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 12/16/13.
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

#import <sstream>
#import "Dictionary_Android.h"
#import "WhirlyKitLog.h"
#import "DictionaryC.h"

namespace WhirlyKit
{

// from DictionaryC
extern RGBAColor ARGBtoRGBAColor(uint32_t v);
extern RGBAColor parseColor(const char* p, RGBAColor ret);

MutableDictionaryRef MutableDictionaryMake()
{
    return std::make_shared<MutableDictionary_Android>();
}

template <typename T>
static std::string genericToString(T val)
{
    std::ostringstream stream;
    stream << val;
    return stream.str();
}
template <typename T>
static T genericFromString(std::string const &string, T defValue)
{
    std::stringstream stream(string);
    T result;
    return (stream >> result) ? result : defValue;
}

int MutableDictionary_Android::StringValue::asInt() const                          { return genericFromString<int>(val, 0); }
int64_t MutableDictionary_Android::StringValue::asInt64() const                    { return genericFromString<int64_t>(val, 0); }
SimpleIdentity MutableDictionary_Android::StringValue::asIdentity() const          { return genericFromString<SimpleIdentity >(val, 0); }
double MutableDictionary_Android::StringValue::asDouble() const                    { return genericFromString<double>(val, 0.0); }
void MutableDictionary_Android::IntValue::asString(std::string &retStr) const      { retStr = genericToString(val); }
std::string MutableDictionary_Android::IntValue::asString() const                  { return genericToString(val); }
void MutableDictionary_Android::Int64Value::asString(std::string &retStr) const    { retStr = genericToString(val); }
std::string MutableDictionary_Android::Int64Value::asString() const                { return genericToString(val); }
void MutableDictionary_Android::DoubleValue::asString(std::string &retStr) const   { retStr = genericToString(val); }
std::string MutableDictionary_Android::DoubleValue::asString() const               { return genericToString(val); }
void MutableDictionary_Android::IdentityValue::asString(std::string &retStr) const { retStr = genericToString(val); }
std::string MutableDictionary_Android::IdentityValue::asString() const             { return genericToString(val); }

MutableDictionary_Android::ArrayValue::ArrayValue(const std::vector<DictionaryEntryRef> &entries)
{
    for (const auto &entry : entries)
    {
        if (const auto valRef = makeValueRef(std::dynamic_pointer_cast<DictionaryEntry_Android>(entry)))
        {
            val.push_back(valRef);
        }
        else
        {
            wkLogLevel(Warn, "Unsupported entry type");
        }
    }
}

MutableDictionary_Android::ArrayValue::ArrayValue(const std::vector<DictionaryRef> &entries)
{
    for (const auto &entry : entries)
    {
        if (const auto valRef = std::make_shared<DictionaryValue>(std::dynamic_pointer_cast<MutableDictionary_Android>(entry)))
        {
           val.push_back(valRef);
        }
        else
        {
            wkLogLevel(Warn, "Unsupported entry type");
        }
    }
}

bool MutableDictionary_Android::IntValue::isEqual(const DictionaryEntry_Android& other) const
{
    return val == other.getInt();
}
bool MutableDictionary_Android::Int64Value::isEqual(const DictionaryEntry_Android& other) const
{
    return val == other.getInt64();
}
bool MutableDictionary_Android::IdentityValue::isEqual(const DictionaryEntry_Android& other) const
{
    return val == other.getIdentity();
}
bool MutableDictionary_Android::DoubleValue::isEqual(const DictionaryEntry_Android& other) const
{
    return val == other.getDouble();
}

bool MutableDictionary_Android::StringValue::isEqual(const DictionaryEntry& other) const
{
    switch (other.getType())
    {
        case DictTypeString:
            if (const auto p = dynamic_cast<const DictionaryEntryCString*>(&other))
            {
                return val == p->getStringRef();
            }
        case DictTypeInt:
        case DictTypeInt64:
        case DictTypeIdentity:
            return val == other.getString();
        case DictTypeDouble:
            // todo: should really parse to double and compare with epsilon
            return val == other.getString();
        case DictTypeObject:
        case DictTypeDictionary:
        case DictTypeArray:
        default:
            return false;
    }
}
bool MutableDictionary_Android::StringValue::isEqual(const DictionaryEntry_Android& other) const
{
    switch (other.getType())
    {
        case DictTypeString:
            if (const auto p = dynamic_cast<MutableDictionary_Android::StringValue*>(other.getValue().get()))
            {
                return val == p->val;
            }
        case DictTypeInt:
        case DictTypeInt64:
        case DictTypeIdentity:
            return val == other.getString();
        case DictTypeDouble:
            // todo: should really parse to double and compare with epsilon
            return val == other.getString();
        case DictTypeObject:
        case DictTypeDictionary:
        case DictTypeArray:
        default:
            return false;
    }
}

MutableDictionaryRef MutableDictionary_Android::copy() const
{
    return std::make_shared<MutableDictionary_Android>(*this);
}

MutableDictionary_Android::MutableDictionary_Android(const MutableDictionary_Android &that)
{
    for (const auto &kv : that.fields)
    {
        fields[kv.first] = kv.second->copy();
    }
}

MutableDictionary_Android::MutableDictionary_Android(MutableDictionary_Android &&that) noexcept
    : fields(std::move(that.fields))
{
}

MutableDictionary_Android::MutableDictionary_Android(const Dictionary &that)
{
    for (const auto &key : that.getKeys())
    {
        setEntry(key,that.getEntry(key));
    }
}

bool MutableDictionary_Android::parseJSON(const std::string &jsonString)
{
    JSONNode topNode = libjson::parse(jsonString);
    return parseJSONNode(topNode);
}

MutableDictionary_Android::ValueRef MutableDictionary_Android::parseJSONValue(JSONNode::iterator &nodeIt)
{
    switch (nodeIt->type())
    {
        case JSON_NULL:     return ValueRef();
        case JSON_STRING:   return std::make_shared<StringValue>(nodeIt->as_string());
        case JSON_NUMBER:   return std::make_shared<DoubleValue>(nodeIt->as_float());
        case JSON_BOOL:     return std::make_shared<IntValue>(nodeIt->as_bool());
        case JSON_ARRAY:
        {
            auto nodes = nodeIt->as_array();
            std::vector<ValueRef> values;
            values.reserve(nodes.size());
            for (auto arrNodeIt = nodes.begin(); arrNodeIt != nodes.end(); ++arrNodeIt)
            {
                values.push_back(parseJSONValue(arrNodeIt));
            }
            return std::make_shared<ArrayValue>(values);
        }
        case JSON_NODE:
        {
            auto dict = std::make_shared<MutableDictionary_Android>();
            auto node = nodeIt->as_node();
            dict->parseJSONNode(node);
            return std::make_shared<DictionaryValue>(dict);
        }
        default:
            wkLogLevel(Warn, "Unsupported type conversion from type %d to JSON", nodeIt->type());
    }

    return ValueRef();
}

bool MutableDictionary_Android::parseJSONNode(JSONNode &node)
{
    for (auto nodeIt = node.begin(); nodeIt != node.end(); ++nodeIt) {
        const auto name = nodeIt->name();
        const auto val = parseJSONValue(nodeIt);
        if (name.empty() || !val)
        {
            return false;
        }
        fields[name] = val;
    }
    return true;
}

void MutableDictionary_Android::clear()
{
    fields.clear();
}
    
MutableDictionary_Android &MutableDictionary_Android::operator=(const MutableDictionary_Android &that)
{
    if (this != &that)
    {
        clear();
        for (const auto &kv : that.fields)
        {
            fields[kv.first] = kv.second->copy();
        }
    }
    return *this;
}

MutableDictionary_Android &MutableDictionary_Android::operator=(MutableDictionary_Android &&that) noexcept
{
    if (this != &that)
    {
        clear();
        fields = std::move(that.fields);
    }
    return *this;
}

MutableDictionary_Android::MutableDictionary_Android(RawData *rawData)
{
    RawDataReader dataRead(rawData);
    while (!dataRead.done())
    {
        int type;
        if (!dataRead.getInt(type))
        {
            wkLogLevel(Warn, "Unable to parse dictionary: no type");
            return;
        }
        std::string attrName;
        if (!dataRead.getString(attrName))
        {
            wkLogLevel(Warn, "Unable to parse: no attribute name");
            return;
        }
        switch (type)
        {
            case DictTypeString:
            {
                std::string sVal;
                if (!dataRead.getString(sVal))
                {
                    wkLogLevel(Warn, "Unable to parse: no string value");
                    return;
                }
                // N.B.: virtual functions don't work from constructors
                this->MutableDictionary_Android::setString(attrName, sVal);
                break;
            }
            case DictTypeInt:
            {
                int iVal;
                if (!dataRead.getInt(iVal))
                {
                    wkLogLevel(Warn, "Unable to parse: no int value");
                    return;
                }
                this->MutableDictionary_Android::setInt(attrName, iVal);
                break;
            }
            case DictTypeInt64:
            {
                int64_t iVal;
                if (!dataRead.getInt64(iVal))
                {
                    wkLogLevel(Warn, "Unable to parse: no int64 value");
                    return;
                }
                this->MutableDictionary_Android::setInt64(attrName, iVal);
                break;
            }
            case DictTypeDouble:
            {
                double dVal;
                if (!dataRead.getDouble(dVal))
                {
                    wkLogLevel(Warn, "Unable to parse: no double value");
                    return;
                }
                this->MutableDictionary_Android::setDouble(attrName, dVal);
                break;
            }
            default:
                wkLogLevel(Warn, "Unrecognized dictionary type %d", type);
                return;
        }
    }
}
    
void MutableDictionary_Android::asRawData(MutableRawData *rawData)
{
    for (const auto &kv : fields)
    {
        auto const &val = kv.second;
        if (val->type() == DictTypeObject)
        {
            wkLogLevel(Warn, "Unsupported entry type %d", val->type());
            continue;
        }
        rawData->addInt(val->type());
        rawData->addString(kv.first);
        switch (val->type())
        {
            case DictTypeString:
            {
                std::string str;
                val->asString(str);
                rawData->addString(str);
                break;
            }
            case DictTypeInt:
                rawData->addInt(val->asInt());
                break;
            case DictTypeInt64:
                rawData->addInt64(val->asInt64());
                break;
            case DictTypeDouble:
                rawData->addDouble(val->asDouble());
                break;
            default:
                assert(!"Unsupported type");
        }
    }
}
    
int MutableDictionary_Android::numFields() const
{
    return (int)fields.size();
}

bool MutableDictionary_Android::hasField(const std::string &name) const
{
    const auto it = fields.find(name);
    return (it != fields.end());
}

DictionaryType MutableDictionary_Android::getType(const std::string &name) const
{
    const auto it = fields.find(name);
    return (it == fields.end()) ? DictTypeNone : it->second->type();
}

void MutableDictionary_Android::removeField(const std::string &name)
{
    const auto it = fields.find(name);
    if (it != fields.end())
    {
        fields.erase(it);
    }
}

int MutableDictionary_Android::getInt(const std::string &name,int defVal) const
{
    const auto it = fields.find(name);
    return (it == fields.end()) ? defVal : it->second->asInt();
}

int64_t MutableDictionary_Android::getInt64(const std::string &name,int64_t defVal) const
{
    const auto it = fields.find(name);
    return (it == fields.end()) ? defVal : it->second->asInt();
}

SimpleIdentity MutableDictionary_Android::getIdentity(const std::string &name) const
{
    const auto it = fields.find(name);
    return (it == fields.end()) ? EmptyIdentity : it->second->asIdentity();
}

bool MutableDictionary_Android::getBool(const std::string &name,bool defVal) const
{
    const auto it = fields.find(name);
    return (it == fields.end()) ? defVal : (it->second->asInt() != 0);
}

RGBAColor MutableDictionary_Android::getColor(const std::string &name,const RGBAColor &defVal) const
{
    const auto it = fields.find(name);
    if (it == fields.end())
    {
        return defVal;
    }

    const Value &val = *it->second;

    switch (val.type())
    {
        case DictTypeString:
        {
            std::string str;
            it->second->asString(str);
            return parseColor(str.c_str(), defVal);
        }
        case DictTypeInt:  return ARGBtoRGBAColor(it->second->asInt());
        default:
            wkLogLevel(Warn, "Unhandled conversion from type %d to color", val.type());
            return defVal;
    }
}

double MutableDictionary_Android::getDouble(const std::string &name,double defVal) const
{
    const auto it = fields.find(name);
    return (it == fields.end()) ? defVal : it->second->asDouble();
}
    
std::string MutableDictionary_Android::getString(const std::string &name) const
{
    const auto it = fields.find(name);
    return (it == fields.end()) ? std::string() : it->second->asString();
}

std::string MutableDictionary_Android::getString(const std::string &name,const std::string &defVal) const
{
    auto const it = fields.find(name);
    return (it == fields.end()) ? defVal : it->second->asString();
}

DelayedDeletableRef MutableDictionary_Android::getObject(const std::string &name)
{
    const auto it = fields.find(name);
    return (it == fields.end()) ? DelayedDeletableRef() : it->second->asObject();
}

DictionaryRef MutableDictionary_Android::getDict(const std::string &name) const
{
    const auto it = fields.find(name);
    if (it != fields.end())
    {
        if (const auto dictVal = dynamic_cast<DictionaryValue *>(it->second.get()))
        {
            return dictVal->val;
        }
        else if (const auto val = dynamic_cast<Value *>(it->second.get()))
        {
            wkLogLevel(Warn, "Unsupported entry type %d for entry '%s'", val->type(), name.c_str());
        }
        else if (it->second)
        {
            wkLogLevel(Warn, "Unsupported entry type ? for entry '%s'", name.c_str());
        }
    }
    return DictionaryRef();
}

DictionaryEntryRef MutableDictionary_Android::getEntry(const std::string &name) const
{
    const auto it = fields.find(name);
    return (it == fields.end()) ? DictionaryEntryRef() : std::make_shared<DictionaryEntry_Android>(it->second);
}

std::vector<DictionaryEntryRef> MutableDictionary_Android::getArray(const std::string &name) const
{
    const auto it = fields.find(name);
    if (it != fields.end())
    {
        const auto gval = it->second.get();
        if (gval && gval->type() == DictTypeArray)
        {
            if (const auto val = dynamic_cast<ArrayValue*>(gval))
            {
                std::vector<DictionaryEntryRef> ret;
                ret.reserve(val->val.size());
                for (const auto &entry : val->val)
                {
                    ret.push_back(std::make_shared<DictionaryEntry_Android>(entry));
                }
                return ret;
            }
            wkLogLevel(Warn,"Unsupported conversion to array");
        }
        wkLogLevel(Warn,"Unsupported conversion from type %d to array", gval->type());
    }
    return std::vector<DictionaryEntryRef>();
}

std::vector<std::string> MutableDictionary_Android::getKeys() const
{
    std::vector<std::string> keys;
    keys.reserve(fields.size());
    for (const auto &it : fields)
    {
        keys.push_back(it.first);
    }

    return keys;
}

void MutableDictionary_Android::setInt(const std::string &name,int val)
{
    removeField(name);
    fields[name] = std::make_shared<IntValue>(val);
}

void MutableDictionary_Android::setInt64(const std::string &name,int64_t val)
{
    removeField(name);
    fields[name] = std::make_shared<Int64Value>(val);
}

void MutableDictionary_Android::setIdentifiable(const std::string &name,SimpleIdentity val)
{
    removeField(name);
    fields[name] = std::make_shared<IdentityValue>(val);
}

void MutableDictionary_Android::setDouble(const std::string &name,double val)
{
    removeField(name);
    fields[name] = std::make_shared<DoubleValue>(val);
}

void MutableDictionary_Android::setString(const std::string &name,const std::string &val)
{
    removeField(name);
    fields[name] = std::make_shared<StringValue>(val);
}

void MutableDictionary_Android::setDict(const std::string &name,const MutableDictionary_AndroidRef &dict)
{
    removeField(name);
    fields[name] = std::make_shared<DictionaryValue>(dict);
}

MutableDictionary_Android::ValueRef MutableDictionary_Android::makeValueRef(const DictionaryEntry_AndroidRef &entry)
{
    switch(entry->getType())
    {
        case DictTypeArray: {
            const auto arr = entry->getArray();
            std::vector<DictionaryEntryRef> entries;
            entries.reserve(arr.size());
            for (const auto &thisEntry : arr)
            {
                entries.push_back(thisEntry);
            }
            return std::make_shared<ArrayValue>(std::move(entries));
        }
        case DictTypeDictionary:
        {
            if (const auto e = std::dynamic_pointer_cast<MutableDictionary_Android>(entry->getDict()))
            {
                return std::make_shared<DictionaryValue>(e);
            }
            wkLogLevel(Warn, "Unsupported dictionary conversion");
            return ValueRef();
        }
        case DictTypeIdentity: return std::make_shared<IdentityValue>(entry->getIdentity());
        case DictTypeInt:      return std::make_shared<IntValue>(entry->getInt());
        case DictTypeInt64:    return std::make_shared<Int64Value>(entry->getInt64());
        case DictTypeDouble:   return std::make_shared<DoubleValue>(entry->getDouble());
        case DictTypeString:   return std::make_shared<StringValue>(entry->getString());
        default:
            wkLogLevel(Warn, "Unsupported conversion from type %d", entry->getType());
            return ValueRef();
    }
}

MutableDictionary_Android::ValueRef MutableDictionary_Android::makeValueRef(const DictionaryEntryRef &entry)
{
    switch(entry->getType())
    {
        case DictTypeArray:
        {
            const auto arr = entry->getArray();
            std::vector<DictionaryEntryRef> entries;
            entries.reserve(arr.size());
            for (const auto &thisEntry : arr) {
                entries.push_back(thisEntry);
            }
            return std::make_shared<ArrayValue>(std::move(entries));
        }
        case DictTypeDictionary:
        {
            if (const auto e = std::dynamic_pointer_cast<MutableDictionary_Android>(entry->getDict()))
            {
                return std::make_shared<DictionaryValue>(e);
            }
            wkLogLevel(Warn, "Unsupported dictionary conversion");
            return ValueRef();
        }
        case DictTypeIdentity: return std::make_shared<IdentityValue>(entry->getIdentity());
        case DictTypeInt:      return std::make_shared<IntValue>(entry->getInt());
        case DictTypeInt64:    return std::make_shared<Int64Value>(entry->getIdentity());
        case DictTypeDouble:   return std::make_shared<DoubleValue>(entry->getDouble());
        case DictTypeString:   return std::make_shared<StringValue>(entry->getString());
        default:
            wkLogLevel(Warn, "Unsupported conversion from type %d", entry->getType());
            return ValueRef();
    }
}

void MutableDictionary_Android::setEntry(const std::string &name,const DictionaryEntryRef &entry)
{
    removeField(name);
    fields[name] = makeValueRef(entry);
}

void MutableDictionary_Android::setArray(const std::string &name,const std::vector<DictionaryEntryRef> &entries)
{
    removeField(name);
    fields[name] = std::make_shared<ArrayValue>(entries);
}

void MutableDictionary_Android::setArray(const std::string &name,const std::vector<DictionaryRef> &entries)
{
    removeField(name);
    fields[name] = std::make_shared<ArrayValue>(entries);
}

void MutableDictionary_Android::setObject(const std::string &name, const DelayedDeletableRef &obj)
{
    removeField(name);
    fields[name] = std::make_shared<ObjectValue>(obj);
}

std::string MutableDictionary_Android::toString() const
{
    std::string str;
    str.reserve(numFields() * 10);
    for (const auto &it : fields)
    {
        const auto valStr = it.second->asString();
        str.reserve(str.length() + it.first.length() + valStr.length() + 2);
        str.append(it.first);
        str.append(":");
        str.append(valStr);
        str.append("\n");
    }
    return str;
}

void MutableDictionary_Android::addEntries(const Dictionary *inOther)
{
    if (const auto other = dynamic_cast<const MutableDictionary_Android *>(inOther))
    {
        for (const auto &kv : other->fields)
        {
            fields[kv.first] = kv.second->copy();
        }
    }
    else
    {
        wkLogLevel(Warn, "Unsupported dictionary type");
    }
}

RGBAColor DictionaryEntry_Android::getColor() const
{
    switch (type)
    {
        case DictTypeString: return parseColor(val->asString().c_str(), RGBAColor::white());
        case DictTypeInt:    return ARGBtoRGBAColor(val->asInt());
        default:
            wkLogLevel(Warn, "Unsupported conversion from type %d to color", type);
            return RGBAColor::white();
    }
}

std::vector<DictionaryEntryRef> DictionaryEntry_Android::getArray() const
{
    if (type == DictTypeArray)
    {
        if (const auto theVal = dynamic_cast<MutableDictionary_Android::ArrayValue*>(val.get()))
        {
            std::vector<DictionaryEntryRef> ret;
            ret.reserve(theVal->val.size());
            for (auto entry: theVal->val)
            {
                ret.push_back(std::make_shared<DictionaryEntry_Android>(entry));
            }
            return ret;
        }
        else if (val)
        {
            wkLogLevel(Warn, "Unsupported conversion to array");
        }
    }
    else
    {
        wkLogLevel(Warn, "Unsupported conversion from type %d to array", type);
    }

    return std::vector<DictionaryEntryRef>();
}

bool DictionaryEntry_Android::isEqual(const DictionaryEntryRef &inOther) const
{
    if (const auto other = dynamic_cast<DictionaryEntry_Android*>(inOther.get()))
    {
        return val->isEqual(other->val);
    }
    else if (const auto other = dynamic_cast<DictionaryEntry*>(inOther.get()))
    {
        return val->isEqual(*other);
    }
    else if (inOther)
    {
        wkLogLevel(Warn, "Unsupported dictionary entry comparison");
    }
    return false;
}

}
