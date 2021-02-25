/*
 *  Dictionary_NSDictionary.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/24/19.
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

#import <Foundation/Foundation.h>
#import "Dictionary.h"
#import "DictionaryC.h"

namespace WhirlyKit
{

class iosDictionary;
typedef std::shared_ptr<iosDictionary> iosDictionaryRef;

/// Wrapper around a single value
class iosDictionaryEntry : public DictionaryEntry
{
public:
    iosDictionaryEntry(id value);
    
    /// Returns the field type
    virtual DictionaryType getType() const;
    /// Return an int, using the default if it's missing
    virtual int getInt() const;
    /// Return a 64 bit unique identity or 0 if missing
    virtual SimpleIdentity getIdentity() const;
    /// Interpret an int as a boolean
    virtual bool getBool() const;
    /// Interpret an int as a RGBA color
    virtual RGBAColor getColor() const;
    /// Return a double, using the default if it's missing
    virtual double getDouble() const;
    /// Return a string, or empty if it's missing
    virtual std::string getString() const;
    /// Return a dictionary as an entry
    virtual DictionaryRef getDict() const;
    /// Return an array of refs
    virtual std::vector<DictionaryEntryRef> getArray() const;
    /// Compare to other
    virtual bool isEqual(const DictionaryEntryRef &other) const;
protected:
    id value;
};

/// The Dictionary is my cross platform replacement for NSDictionary
/// On iOS its just a wrapper
class iosDictionary : public Dictionary
{
public:
    iosDictionary();
    iosDictionary(NSDictionary *dict);
    // Copy constructor
    iosDictionary(const iosDictionary &that);
    virtual ~iosDictionary() override;
    
    /// Returns true if the field exists
    bool hasField(const std::string &name) const override;
    
    /// Returns the field type
    DictionaryType getType(const std::string &name) const override;
    
    /// Return an int, using the default if it's missing
    int getInt(const std::string &name,int defVal=0) const override;
    /// Return an int64, using the default if it's missing
    int64_t getInt64(const std::string &name, int64_t defVal) const override;
    /// Return a 64 bit unique identity or 0 if missing
    SimpleIdentity getIdentity(const std::string &name) const override;
    /// Interpret an int as a boolean
    bool getBool(const std::string &name,bool defVal=false) const override;
    /// Interpret an int as a RGBA color
    RGBAColor getColor(const std::string &name,const RGBAColor &defVal) const override;
    /// Return a double, using the default if it's missing
    double getDouble(const std::string &name,double defVal=0.0) const override;
    /// Return a string, or empty if it's missing
    std::string getString(const std::string &name) const override;
    /// Return a string, using the default if it's missing
    std::string getString(const std::string &name,const std::string &defVal) const override;
    /// Return a dictionary as an entry
    DictionaryRef getDict(const std::string &name) const override;
    // Return a generic entry
    virtual DictionaryEntryRef getEntry(const std::string &name) const override;
    // Return an array (if it is an array)
    virtual std::vector<DictionaryEntryRef> getArray(const std::string &name) const override;
    // Return an array of key names
    virtual std::vector<std::string> getKeys() const override;
public:
    NSDictionary *dict;
};

class iosMutableDictionary;
typedef std::shared_ptr<iosMutableDictionary> iosMutableDictionaryRef;

/// This version of the dictionary can be modified
class iosMutableDictionary : public MutableDictionary
{
public:
    iosMutableDictionary();
    iosMutableDictionary(NSMutableDictionary *dict);
    iosMutableDictionary(MutableDictionaryRef dict);
    // Assignment operator
    virtual iosMutableDictionary &operator = (const iosMutableDictionary &that);
    virtual ~iosMutableDictionary() override;
    virtual MutableDictionaryRef copy() const override;

    /// Returns true if the field exists
    bool hasField(const std::string &name) const override;
    
    /// Returns the field type
    DictionaryType getType(const std::string &name) const override;
    
    /// Return an int, using the default if it's missing
    int getInt(const std::string &name,int defVal=0.0) const override;
    /// Return an int64, using the default if it's missing
    int64_t getInt64(const std::string &name, int64_t defVal) const override;
    /// Return a 64 bit unique identity or 0 if missing
    SimpleIdentity getIdentity(const std::string &name) const override;
    /// Interpret an int as a boolean
    bool getBool(const std::string &name,bool defVal=false) const override;
    /// Interpret an int as a RGBA color
    RGBAColor getColor(const std::string &name,const RGBAColor &defVal) const override;
    /// Return a double, using the default if it's missing
    double getDouble(const std::string &name,double defVal=0.0) const override;
    /// Return a string, or empty if it's missing
    std::string getString(const std::string &name) const override;
    /// Return a string, using the default if it's missing
    std::string getString(const std::string &name,const std::string &defVal) const override;
    /// Return a dictionary as an entry
    DictionaryRef getDict(const std::string &name) const override;
    // Return a generic entry
    DictionaryEntryRef getEntry(const std::string &name) const override;
    // Return an array (if it is an array)
    std::vector<DictionaryEntryRef> getArray(const std::string &name) const override;
    // Return an array of key names
    virtual std::vector<std::string> getKeys() const override;

    /// Clean out the contents
    void clear() override;
    
    /// Remove the given field by name
    void removeField(const std::string &name) override;
    
    /// Set field as int
    void setInt(const std::string &name,int val) override;
    /// Set field as 64 bit unique value
    void setIdentifiable(const std::string &name,SimpleIdentity val) override;
    /// Set field as double
    void setDouble(const std::string &name,double val) override;
    /// Set field as string
    void setString(const std::string &name,const std::string &val) override;
    
    // Merge in key-value pairs from another dictionary
    void addEntries(const Dictionary *other) override;
    
public:
    NSMutableDictionary *dict;
};

}

// Convert an NSDictionary to a C++ native DictionaryC
@interface NSDictionary (DictionaryC)

// Convert to the native C++ DictionaryC
- (WhirlyKit::MutableDictionaryCRef) toDictionaryC;

@end

@interface NSMutableDictionary (DictionaryC)

// Convert one of the DictionaryC objects to an NSDictionary (actually works on dictionary in the right interface)
+ (NSMutableDictionary *) fromDictionaryC:(WhirlyKit::MutableDictionaryRef)dict;
+ (NSMutableDictionary *) fromDictionaryCPointer:(const WhirlyKit::MutableDictionary *)dict;

@end
