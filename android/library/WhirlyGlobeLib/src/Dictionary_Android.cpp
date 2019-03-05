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

void MutableDictionary_Android::clear()
{
    for (FieldMap::iterator it = fields.begin();it != fields.end();++it)
        delete it->second;
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
        Value *val = it->second;
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
    {
        delete it->second;
        fields.erase(it);
    }
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

void MutableDictionary_Android::setInt(const std::string &name,int val)
{
    removeField(name);
    
    IntValue *iVal = new IntValue();
    iVal->val = val;
    fields[name] = iVal;
}
    
void MutableDictionary_Android::setIdentifiable(const std::string &name,SimpleIdentity val)
{
    removeField(name);
    
    IdentityValue *iVal = new IdentityValue();
    iVal->val = val;
    fields[name] = iVal;
}

void MutableDictionary_Android::setDouble(const std::string &name,double val)
{
    removeField(name);
    
    DoubleValue *dVal = new DoubleValue();
    dVal->val = val;
    fields[name] = dVal;
}

void MutableDictionary_Android::setString(const std::string &name,const std::string &val)
{
    removeField(name);
    
    StringValue *sVal = new StringValue();
    sVal->val = val;
    fields[name] = sVal;
}
    
void MutableDictionary_Android::setObject(const std::string &name, DelayedDeletableRef obj)
{
    removeField(name);
    
    ObjectValue *oVal = new ObjectValue();
    oVal->val = obj;
    fields[name] = oVal;
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
    
}
