/*
 *  Dictionary.h
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

#import <map>
#import <unordered_map>
#import <string>
#import "WhirlyVector.h"
#import "CoordSystem.h"
#import "RawData.h"
#import "Dictionary.h"
#import "libjson.h"

namespace WhirlyKit
{
class MutableDictionaryC;
typedef std::shared_ptr<MutableDictionaryC> MutableDictionaryCRef;

class DictionaryEntryC;
typedef std::shared_ptr<DictionaryEntryC> DictionaryEntryCRef;

/// The Dictionary is my cross platform replacement for NSDictionary
/// TODO: Removing & adding things repeatedly will just cause this to grow
class MutableDictionaryC : public MutableDictionary
{
public:
    MutableDictionaryC();
    MutableDictionaryC(int capacity);
    // Construct from a raw data buffer
//    MutableDictionaryC(RawData *rawData);
    // Copy constructor
    MutableDictionaryC(const MutableDictionaryC &that);
    // Move constructor
    MutableDictionaryC(MutableDictionaryC &&that) noexcept;
    // Assignment operator
    MutableDictionaryC &operator = (const MutableDictionaryC &that);
    // Move assignment operator
    MutableDictionaryC &operator = (MutableDictionaryC &&that) noexcept;
    virtual ~MutableDictionaryC() = default;

    virtual MutableDictionaryRef copy() const override { return std::make_shared<MutableDictionaryC>(*this); }

    // Parse from a JSON string
//    bool parseJSON(const std::string jsonString);
//    bool parseJSONNode(JSONNode &node);
//    ValueRef parseJSONValue(JSONNode::iterator &nodeIt);

    virtual int count() const override { return numFields(); }
    virtual bool empty() const override { return numFields() == 0; }

    /// Clean out the contents
    void clear() override;
    
    /// Number of fields being represented
    int numFields() const { return valueMap.size(); }

    /// Returns true if the field exists
    virtual bool hasField(const std::string &name) const override;
    virtual bool hasField(unsigned int key) const;

    /// Returns the field type
    virtual DictionaryType getType(const std::string &name) const override;
    virtual DictionaryType getType(unsigned int key) const;

    /// Remove the given field by name
    void removeField(const std::string &name) override;
    void removeField(unsigned int key);

    /// Return an int, using the default if it's missing
    virtual int getInt(const std::string &name,int defVal) const override;
    virtual int getInt(unsigned int key,int defVal) const;
    /// Return a 64 bit unique identity or 0 if missing
    virtual SimpleIdentity getIdentity(const std::string &name) const override;
    virtual SimpleIdentity getIdentity(unsigned int key) const;
    /// Return a 64 bit value or 0 if missing
    virtual int64_t getInt64(const std::string &name,int64_t defVal) const override;
    virtual int64_t getInt64(unsigned int key,int64_t defVal) const;
    /// Interpret an int as a boolean
    virtual bool getBool(const std::string &name,bool defVal) const override;
    virtual bool getBool(unsigned int key,bool defVal) const;
    /// Interpret an int as a RGBA color
    virtual RGBAColor getColor(const std::string &name,const RGBAColor &defVal) const override;
    virtual RGBAColor getColor(unsigned int key,const RGBAColor &defVal) const;
    /// Return a double, using the default if it's missing
    virtual double getDouble(const std::string &name,double defVal) const override;
    virtual double getDouble(unsigned int key,double defVal) const;
    /// Return a string, or empty if it's missing
    virtual std::string getString(const std::string &name) const override;
    virtual std::string getString(unsigned int key) const;
    /// Return a string, using the default if it's missing
    virtual std::string getString(const std::string &name,const std::string &defVal) const override;
    virtual std::string getString(unsigned int key,const std::string &defVal) const;
    /// Return a dictionary as an entry
    virtual DictionaryRef getDict(const std::string &name) const override;
    virtual DictionaryRef getDict(unsigned int key) const;
    // Return a generic entry
    virtual DictionaryEntryRef getEntry(const std::string &name) const override;
    virtual DictionaryEntryRef getEntry(unsigned int key) const;
    // Return an array (if it is an array)
    virtual std::vector<DictionaryEntryRef> getArray(const std::string &name) const override;
    virtual std::vector<DictionaryEntryRef> getArray(unsigned int key) const;
    // Return an array of keys
    virtual std::vector<std::string> getKeys() const override;

    /// Get the key for the given string
    int getKeyID(const std::string &name);
    
    /// Set field as int
    void setInt(const std::string &name,int val) override;
    void setInt(unsigned int key,int val);
    /// Set field as int64
    virtual void setInt64(const std::string &name,int64_t val) override;
    virtual void setInt64(unsigned int key,int64_t val);
    /// Set field as 64 bit unique value
    void setIdentifiable(const std::string &name,SimpleIdentity val) override;
    void setIdentifiable(unsigned int key,SimpleIdentity val);
    /// Set field as double
    void setDouble(const std::string &name,double val) override;
    void setDouble(unsigned int key,double val);
    /// Set field as string
    void setString(const std::string &name,const std::string &val) override;
    void setString(unsigned int key,const std::string &val);
    /// Set the dictionary at the given attribute name
    void setDict(const std::string &name,const MutableDictionaryCRef &dict);
    void setDict(unsigned int key,const MutableDictionaryCRef &dict);
    /// Set the array at the given attribute name
    void setArray(const std::string &name,const std::vector<DictionaryEntryRef> &entries);
    void setArray(unsigned int key,const std::vector<DictionaryEntryRef> &entries);
    /// Set the array at the given attribute name
    void setArray(const std::string &name,const std::vector<DictionaryRef> &entries);
    void setArray(unsigned int key,const std::vector<DictionaryRef> &entries);
    
    // Write data to a raw data buffer
//    void asRawData(MutableRawData *rawData);
    
    // Merge in key-value pairs from another dictionary
    void addEntries(const Dictionary *other) override;
    void addEntries(const MutableDictionaryC *other);
    
protected:
    // The array has to let us know what the type is
    class ArrayEntry {
    public:
        DictionaryType type;   // Type of the data in the array
        std::vector<unsigned int> vals;  // Array entries
    };

    /// Add the given string key
    unsigned int addKeyID(const std::string &name) { return addString(name); }
    unsigned int addString(const std::string &name);
    /// Form an array of entries from an array index;
    std::vector<DictionaryEntryCRef> formArray(int idx) const;

    // The top level entries in the dictionary
    struct Value {
        Value() : type(DictTypeNone), entry(0) { }
        Value & operator = (const Value &other) { type = other.type;  entry = other.entry;  return *this; }
        Value(const Value &other) : type(other.type), entry(other.entry) {}
        Value(DictionaryType type,unsigned int entry) : type(type), entry(entry) {}

        DictionaryType type;
        unsigned int entry;
    };

    // Make an entry ref for a given value
    DictionaryEntryCRef makeEntryRef(const Value &val) const;

    void setupArray(const std::vector<DictionaryEntryCRef> &vals, std::vector<Value>& arr);

    template <typename TVal, typename TVec>
    void set(unsigned int key, TVal val, DictionaryType type, std::vector<TVec> &vals) {
        set(key, val, type, type, vals);
    }
    template <typename TVal, typename TVec>
    void set(unsigned int key, TVal val, DictionaryType type, DictionaryType altType, std::vector<TVec> &vals) {
        const auto res = valueMap.insert(std::make_pair(key, Value(type, (int)vals.size())));
        if (res.second) {
            // key was inserted
            vals.push_back(val);
        } else {
            // key was already present
            const auto &mapVal = res.first->second;
            
            if (mapVal.type != type && mapVal.type != altType) {
                // type mismatch, remove it
                // shouldn't we replace it?
                valueMap.erase(res.first);
            } else {
                vals[mapVal.entry] = val;
            }
        }
    }

    // Where we store the actual data
    std::vector<int> intVals;
    std::vector<int64_t> int64Vals;
    std::vector<double> dVals;
    std::vector<std::string> stringVals;
    std::vector<std::vector<Value> > arrayVals;
    std::vector<MutableDictionaryCRef> dictVals;
    
    // Map strings to integer values for lookup
    typedef std::unordered_map<std::string,unsigned int> StringMap;
    StringMap stringMap;
    
    // TODO: Turn the values into an array and index the string values separately from keys
    // Map integer key values into fields
    typedef std::unordered_map<unsigned int,Value> ValueMap;
    ValueMap valueMap;
};

/// Wrapper around a single value
class DictionaryEntryC : public DictionaryEntry
{
public:
    DictionaryEntryC(DictionaryType type) : type(type) { }
    
    /// Returns the field type
    virtual DictionaryType getType() const override { return type; }
    /// Return an int, using the default if it's missing
    virtual int getInt() const override { return 0; }
    /// Return a 64 bit unique identity or 0 if missing
    virtual SimpleIdentity getIdentity() const override {return 0; }
    /// Return a 64 bit value or 0 if missing
    virtual int64_t getInt64() const { return 0; }
    /// Interpret an int as a boolean
    virtual bool getBool() const override { return false; }
    /// Interpret an int as a RGBA color
    virtual RGBAColor getColor() const override { return RGBAColor(0,0,0,0); }
    /// Return a double, using the default if it's missing
    virtual double getDouble() const override { return 0.0; }
    /// Return a string, or empty if it's missing
    virtual std::string getString() const override { return ""; }
    /// Return a dictionary as an entry
    virtual DictionaryRef getDict() const override { return DictionaryRef(); }
    /// Return an array of refs
    virtual std::vector<DictionaryEntryRef> getArray() const override { return std::vector<DictionaryEntryRef>(); }
    /// Compare to other
    virtual bool isEqual(const DictionaryEntryRef &other) const override { return false; }

protected:
    DictionaryType type;
};
typedef std::shared_ptr<DictionaryEntryC> DictionaryEntryCRef;

/// Simple entries for a dictionary
class DictionaryEntryCBasic : public DictionaryEntryC
{
public:
    DictionaryEntryCBasic(int iVal)     : DictionaryEntryC(DictTypeInt),    val { .iVal   = iVal } { }
    DictionaryEntryCBasic(double dVal)  : DictionaryEntryC(DictTypeDouble), val { .dVal   = dVal } { }
    DictionaryEntryCBasic(int64_t iVal) : DictionaryEntryC(DictTypeInt64),  val { .i64Val = iVal } { }
    virtual ~DictionaryEntryCBasic() = default;

    /// Return an int, using the default if it's missing
    virtual int getInt() const override;
    /// Return a 64 bit unique identity or 0 if missing
    virtual SimpleIdentity getIdentity() const override;
    /// Return a 64 bit value or 0 if missing
    virtual int64_t getInt64() const override;
    /// Interpret an int as a boolean
    virtual bool getBool() const override;
    /// Interpret an int as a RGBA color
    virtual RGBAColor getColor() const override;
    /// Return a double, using the default if it's missing
    virtual double getDouble() const override;
    /// Compare to other
    virtual bool isEqual(const DictionaryEntryRef &other) const override;

public:
    union {
        int iVal;
        double dVal;
        int64_t i64Val;
    } val;
};

/// String entry for a dictionary
class DictionaryEntryCString : public DictionaryEntryC
{
public:
    DictionaryEntryCString(const std::string &str)
        : DictionaryEntryC(DictTypeString), str(str)
    {
    }
    DictionaryEntryCString(std::string &&str)
            : DictionaryEntryC(DictTypeString), str(std::move(str))
    {
    }
    virtual ~DictionaryEntryCString() = default;

    const std::string &getStringRef() const { return str; }

    // TODO: Parse ints and such out of the string
    
    /// Return a string, or empty if it's missing
    virtual std::string getString() const override { return str; }
    /// Interpret an int as a RGBA color
    virtual RGBAColor getColor() const override;

    /// Compare to other
    virtual bool isEqual(const DictionaryEntryRef &other) const override;

protected:
    std::string str;
};

/// Dictionary entry that is, itself a dictionary (Dictionaryception!)
class DictionaryEntryCDict : public DictionaryEntryC
{
public:
    DictionaryEntryCDict(const MutableDictionaryCRef &dict)
        : DictionaryEntryC(DictTypeDictionary), dict(dict)
    {
    }
    DictionaryEntryCDict(MutableDictionaryCRef &&dict)
            : DictionaryEntryC(DictTypeDictionary), dict(std::move(dict))
    {
    }
    virtual ~DictionaryEntryCDict() = default;

    /// Return a dictionary as an entry
    virtual DictionaryRef getDict() const override { return dict; }

    /// Compare to other
    virtual bool isEqual(const DictionaryEntryRef &other) const override;

protected:
    MutableDictionaryCRef dict;
};
typedef std::shared_ptr<DictionaryEntryC> DictionaryEntryCRef;

/// Dictionary entry that contains an array
class DictionaryEntryCArray : public DictionaryEntryC
{
    friend MutableDictionaryC;
public:
    DictionaryEntryCArray(const std::vector<DictionaryEntryCRef> &vals)
        : DictionaryEntryC(DictTypeArray), vals(vals)
    {
    }
    DictionaryEntryCArray(std::vector<DictionaryEntryCRef> &&vals) noexcept
            : DictionaryEntryC(DictTypeArray), vals(std::move(vals))
    {
    }
    DictionaryEntryCArray(const std::vector<DictionaryEntryRef> &vals);
    virtual ~DictionaryEntryCArray() = default;

    /// Return the array
    virtual std::vector<DictionaryEntryRef> getArray() const override;
    virtual std::vector<DictionaryEntryCRef> getArrayC() const;

    /// Compare to other
    virtual bool isEqual(const DictionaryEntryRef &other) const override;

protected:
    std::vector<DictionaryEntryCRef> vals;
};

}
