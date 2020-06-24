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
#import <string>
#import "WhirlyVector.h"
#import "CoordSystem.h"
#import "RawData.h"
#import "Dictionary.h"
#import "libjson.h"

namespace WhirlyKit
{
class MutableDictionary_Android;
typedef std::shared_ptr<MutableDictionary_Android> MutableDictionary_AndroidRef;

class DictionaryEntry_Android;
typedef std::shared_ptr<DictionaryEntry_Android> DictionaryEntry_AndroidRef;

/// The Dictionary is my cross platform replacement for NSDictionary
class MutableDictionary_Android : public MutableDictionary
{
public:
    MutableDictionary_Android();
    // Construct from a raw data buffer
    MutableDictionary_Android(RawData *rawData);
    // Copy constructor
    MutableDictionary_Android(const MutableDictionary_Android &that);
    // Assignment operator
    MutableDictionary_Android &operator = (const MutableDictionary_Android &that);
    virtual MutableDictionaryRef copy();
    virtual ~MutableDictionary_Android();

    class Value;
    typedef std::shared_ptr<Value> ValueRef;

    // Parse from a JSON string
    bool parseJSON(const std::string jsonString);
    bool parseJSONNode(JSONNode &node);
    ValueRef parseJSONValue(JSONNode::iterator &nodeIt);

    /// Clean out the contents
    void clear();
    
    /// Number of fields being represented
    int numFields() const;
    
    /// Returns true if the field exists
    virtual bool hasField(const std::string &name) const;
    
    /// Returns the field type
    virtual DictionaryType getType(const std::string &name) const;
    
    /// Remove the given field by name
    void removeField(const std::string &name);
    
    /// Return an int, using the default if it's missing
    virtual int getInt(const std::string &name,int defVal=0.0) const;
    /// Return a 64 bit unique identity or 0 if missing
    virtual SimpleIdentity getIdentity(const std::string &name) const;
    /// Interpret an int as a boolean
    virtual bool getBool(const std::string &name,bool defVal=false) const;
    /// Interpret an int as a RGBA color
    virtual RGBAColor getColor(const std::string &name,const RGBAColor &defVal) const;
    /// Return a double, using the default if it's missing
    virtual double getDouble(const std::string &name,double defVal=0.0) const;
    /// Return a string, or empty if it's missing
    virtual std::string getString(const std::string &name) const;
    /// Return a string, using the default if it's missing
    virtual std::string getString(const std::string &name,const std::string &defVal) const;
    /// Return an object pointer
    DelayedDeletableRef getObject(const std::string &name);
    /// Return a dictionary as an entry
    virtual DictionaryRef getDict(const std::string &name) const;
    // Return a generic entry
    virtual DictionaryEntryRef getEntry(const std::string &name) const;
    // Return an array (if it is an array)
    virtual std::vector<DictionaryEntryRef> getArray(const std::string &name) const;
    // Return an array of keys
    virtual std::vector<std::string> getKeys() const;

    /// Set field as int
    void setInt(const std::string &name,int val);
    /// Set field as 64 bit unique value
    void setIdentifiable(const std::string &name,SimpleIdentity val);
    /// Set field as double
    void setDouble(const std::string &name,double val);
    /// Set field as string
    void setString(const std::string &name,const std::string &val);
    /// Set the dictionary at the given attribute name
    void setDict(const std::string &name,MutableDictionary_AndroidRef dict);
    /// Set the entry at the given attribute name
    void setEntry(const std::string &name,DictionaryEntry_AndroidRef entry);
    /// Set the array at the given attribute name
    void setArray(const std::string &name,std::vector<DictionaryEntryRef> &entries);
    /// Set the array at the given attribute name
    void setArray(const std::string &name,std::vector<DictionaryRef> &entries);
    /// Set field as pointer
    void setObject(const std::string &name,DelayedDeletableRef obj);
    
    // Write data to a raw data buffer
    void asRawData(MutableRawData *rawData);
    
    // Convert to a string for debugging
    std::string toString() const;

    // Merge in key-value pairs from another dictionary
    void addEntries(const Dictionary *other);

    // Make a generic ValueRef from a generic entry (yeah, they're different
    static ValueRef makeValueRef(DictionaryEntry_AndroidRef entry);

    class Value
    {
    public:
        Value() { }
        virtual ~Value() { }

        virtual DictionaryType type() = 0;
        virtual ValueRef copy() = 0;
        virtual int asInt() = 0;
        virtual SimpleIdentity asIdentity() = 0;
        virtual void asString(std::string &retStr) = 0;
        virtual double asDouble() = 0;
        virtual DelayedDeletableRef asObject() { return DelayedDeletableRef(); }
        virtual DictionaryRef asDict() { return DictionaryRef(); }
    };

    class StringValue : public Value
    {
    public:
        StringValue() { }
        StringValue(const std::string &inVal) : val(inVal) { }

        virtual DictionaryType type() { return DictTypeString; }
        virtual ValueRef copy() { return ValueRef(new StringValue(val)); }
        virtual int asInt();
        virtual SimpleIdentity asIdentity();
        virtual void asString(std::string &retStr) { retStr = val; }
        virtual double asDouble();

        std::string val;
    };

    class IntValue : public Value
    {
    public:
        IntValue() : val(0) { }
        IntValue(int inVal) : val(inVal) { }

        virtual DictionaryType type() { return DictTypeInt; }
        virtual ValueRef copy() { return ValueRef(new IntValue(val)); }
        virtual int asInt() { return val; }
        virtual SimpleIdentity asIdentity() { return val; }
        virtual void asString(std::string &retStr);
        virtual double asDouble() { return (double)val; }

        int val;
    };

    class DoubleValue : public Value
    {
    public:
        DoubleValue() : val(0.0) { }
        DoubleValue(double inVal) : val(inVal) { }

        virtual DictionaryType type() { return DictTypeDouble; }
        virtual ValueRef copy() { return ValueRef(new DoubleValue(val)); }
        virtual int asInt() { return (int)val; }
        virtual SimpleIdentity asIdentity() { return EmptyIdentity; }
        virtual void asString(std::string &retStr);
        virtual double asDouble() { return val; }

        double val;
    };

    class IdentityValue : public Value
    {
    public:
        IdentityValue() : val(0) { }
        IdentityValue(SimpleIdentity inVal) : val(inVal) { }

        virtual DictionaryType type() { return DictTypeIdentity; }
        virtual ValueRef copy() { return ValueRef(new IdentityValue(val)); }
        virtual int asInt() { return (int)val; }
        virtual SimpleIdentity asIdentity() { return val; }
        virtual void asString(std::string &retStr);
        virtual double asDouble() { return (double)val; }

        SimpleIdentity val;
    };

    class ObjectValue : public Value
    {
    public:
        ObjectValue() { }
        ObjectValue(DelayedDeletableRef inVal) : val(inVal) { }

        virtual DictionaryType type() { return DictTypeObject; }
        virtual ValueRef copy() { return ValueRef(new ObjectValue(val)); }
        virtual int asInt() { return 0; }
        virtual void asString(std::string &retStr) { }
        virtual SimpleIdentity asIdentity() { return EmptyIdentity; }
        virtual double asDouble() { return 0.0; }
        virtual DelayedDeletableRef asObject() { return val; }

        DelayedDeletableRef val;
    };

    class DictionaryValue : public Value
    {
    public:
        DictionaryValue() { }
        DictionaryValue(MutableDictionary_AndroidRef inVal) : val(inVal) { }

        virtual DictionaryType type() { return DictTypeDictionary; }
        virtual ValueRef copy() { return ValueRef(new DictionaryValue(val)); }
        virtual int asInt() { return 0; }
        virtual void asString(std::string &retStr) { }
        virtual SimpleIdentity asIdentity() { return EmptyIdentity; }
        virtual double asDouble() { return 0.0; }
        virtual DictionaryRef asDict();

        MutableDictionary_AndroidRef val;
    };
    typedef std::shared_ptr<DictionaryValue> DictionaryValueRef;

    class ArrayValue : public Value
    {
    public:
        ArrayValue() { }
        ~ArrayValue() { }
        ArrayValue(std::vector<ValueRef> &inVal) : val(inVal) { }
        ArrayValue(std::vector<DictionaryEntryRef> &inVal);
        ArrayValue(std::vector<DictionaryRef> &inVal);

        virtual DictionaryType type() { return DictTypeArray; }
        virtual ValueRef copy() { return ValueRef(new ArrayValue(val)); }
        virtual int asInt() { return 0; }
        virtual void asString(std::string &retStr) { }
        virtual SimpleIdentity asIdentity() { return EmptyIdentity; }
        virtual double asDouble() { return 0.0; }

        std::vector<ValueRef> val;
    };
    typedef std::shared_ptr<ArrayValue> ArrayValueRef;

protected:
    typedef std::map<std::string,ValueRef> FieldMap;
    FieldMap fields;
};

/// Wrapper around a single value
class DictionaryEntry_Android : public DictionaryEntry
{
public:
    DictionaryEntry_Android() : type(DictTypeNone) { };
    DictionaryEntry_Android(MutableDictionary_Android::ValueRef val) : val(val) { type = val->type(); }
    DictionaryEntry_Android &operator = (const DictionaryEntry_Android &that) { val = that.val;  type = that.type; return *this; }

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
    virtual bool isEqual(DictionaryEntryRef other) const;

protected:
    DictionaryType type;
    MutableDictionary_Android::ValueRef val;
};

}
