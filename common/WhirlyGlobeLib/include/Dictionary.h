/*
 *  Dictionary.h
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

#import "Identifiable.h"
#import "WhirlyVector.h"
#import "CoordSystem.h"
#import "RawData.h"

namespace WhirlyKit
{
    
/// Data types in dictionary
typedef enum {DictTypeNone,DictTypeString,DictTypeInt,DictTypeIdentity,DictTypeDouble,DictTypeObject,DictTypeDictionary,DictTypeArray} DictionaryType;

class Dictionary;
typedef std::shared_ptr<Dictionary> DictionaryRef;

class DictionaryEntry;
typedef std::shared_ptr<DictionaryEntry> DictionaryEntryRef;

/// The Dictionary is my cross platform replacement for NSDictionary
class Dictionary
{
public:
    Dictionary();
    virtual ~Dictionary() { };

    /// Returns true if the field exists
    virtual bool hasField(const std::string &name) const = 0;
    
    /// Returns the field type
    virtual DictionaryType getType(const std::string &name) const = 0;
    
    /// Return an int, using the default if it's missing
    virtual int getInt(const std::string &name,int defVal=0.0) const = 0;
    /// Return a 64 bit unique identity or 0 if missing
    virtual SimpleIdentity getIdentity(const std::string &name) const = 0;
    /// Interpret an int as a boolean
    virtual bool getBool(const std::string &name,bool defVal=false) const = 0;
    /// Interpret an int as a RGBA color
    virtual RGBAColor getColor(const std::string &name,const RGBAColor &defVal) const = 0;
    /// Return a double, using the default if it's missing
    virtual double getDouble(const std::string &name,double defVal=0.0) const = 0;
    /// Return a string, or empty if it's missing
    virtual std::string getString(const std::string &name) const = 0;
    /// Return a string, using the default if it's missing
    virtual std::string getString(const std::string &name,const std::string &defVal) const = 0;
    /// Return a dictionary as an entry
    virtual DictionaryRef getDict(const std::string &name) const = 0;
    // Return a generic entry
    virtual DictionaryEntryRef getEntry(const std::string &name) const = 0;
    // Return an array (if it is an array)
    virtual std::vector<DictionaryEntryRef> getArray(const std::string &name) const = 0;
    // Return an array of key names
    virtual std::vector<std::string> getKeys() const = 0;
};

class MutableDictionary;
typedef std::shared_ptr<MutableDictionary> MutableDictionaryRef;

// A reference to an entry in the dictionary (kind of a hack)
class DictionaryEntry
{
public:
    /// Returns the field type
    virtual DictionaryType getType() const = 0;
    /// Return an int, using the default if it's missing
    virtual int getInt() const = 0;
    /// Return a 64 bit unique identity or 0 if missing
    virtual SimpleIdentity getIdentity() const = 0;
    /// Interpret an int as a boolean
    virtual bool getBool() const = 0;
    /// Interpret an int as a RGBA color
    virtual RGBAColor getColor() const = 0;
    /// Return a double, using the default if it's missing
    virtual double getDouble() const = 0;
    /// Return a string, or empty if it's missing
    virtual std::string getString() const = 0;
    /// Return a dictionary as an entry
    virtual DictionaryRef getDict() const = 0;
    /// Return an array of refs
    virtual std::vector<DictionaryEntryRef> getArray() const = 0;
    /// Compare to other
    virtual bool isEqual(DictionaryEntryRef other) const = 0;
};

/// This version of the dictionary can be modified
class MutableDictionary : public Dictionary
{
public:
    MutableDictionary();
    // Assignment operator
    virtual MutableDictionary &operator = (const MutableDictionary &that) { return *this; };
    virtual ~MutableDictionary() { };
    // Make a separate copy of this dictionary
    virtual MutableDictionaryRef copy() = 0;

    /// Clean out the contents
    virtual void clear() = 0;

    /// Remove the given field by name
    virtual void removeField(const std::string &name) = 0;

    /// Set field as int
    virtual void setInt(const std::string &name,int val) = 0;
    /// Set field as 64 bit unique value
    virtual void setIdentifiable(const std::string &name,SimpleIdentity val) = 0;
    /// Set field as double
    virtual void setDouble(const std::string &name,double val) = 0;
    /// Set field as string
    virtual void setString(const std::string &name,const std::string &val) = 0;

    // Merge in key-value pairs from another dictionary
    virtual void addEntries(const Dictionary *other) = 0;
};
    
/// Builds a platform appropriate mutable dictionary
extern MutableDictionaryRef MutableDictionaryMake();
    
}
