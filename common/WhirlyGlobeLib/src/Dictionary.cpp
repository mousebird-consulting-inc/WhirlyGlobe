/*
 *  Dictionary.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 12/16/13.
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

#import <sstream>
#import "Dictionary.h"

namespace WhirlyKit
{

Dictionary::Dictionary()
{
}

DictionaryEntryRef Dictionary::getEntry(const std::string &name) const
{
    DictionaryEntry *entry = new DictionaryEntry(this,name);
    return DictionaryEntryRef(entry);
}

MutableDictionary::MutableDictionary()
{
}

DictionaryEntry::DictionaryEntry(const Dictionary *dict,const std::string &name)
: dict(const_cast<Dictionary *>(dict)), name(name)
{
    if (dict)
        type = dict->getType(name);
}

DictionaryType DictionaryEntry::getType() const
{
    return type;
}

int DictionaryEntry::getInt() const
{
    return dict->getInt(name);
}

SimpleIdentity DictionaryEntry::getIdentity() const
{
    return dict->getIdentity(name);
}

bool DictionaryEntry::getBool() const
{
    return dict->getBool(name);
}

RGBAColor DictionaryEntry::getColor() const
{
    return dict->getColor(name, RGBAColor::white());
}

double DictionaryEntry::getDouble() const
{
    return dict->getDouble(name);
}

std::string DictionaryEntry::getString() const
{
    return dict->getString(name);
}

DictionaryRef DictionaryEntry::getDict() const
{
    return dict->getDict(name);
}
    
}
