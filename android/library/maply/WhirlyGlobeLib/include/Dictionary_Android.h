/*  Dictionary.h
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
    MutableDictionary_Android() {}
    // Construct from a raw data buffer
    MutableDictionary_Android(RawData *rawData);
    // Copy constructor
    MutableDictionary_Android(const MutableDictionary_Android &that);
    MutableDictionary_Android(MutableDictionary_Android &&that) noexcept;
    MutableDictionary_Android(const Dictionary &that);
    // Assignment operator
    MutableDictionary_Android &operator=(const MutableDictionary_Android &that);
    MutableDictionary_Android &operator=(MutableDictionary_Android &&that) noexcept;
    virtual MutableDictionaryRef copy() const override;
    virtual ~MutableDictionary_Android() = default;

    struct Value;
    typedef std::shared_ptr<Value> ValueRef;

    // Parse from a JSON string
    bool parseJSON(const std::string &jsonString);
    bool parseJSONNode(JSONNode &node);
    ValueRef parseJSONValue(JSONNode::iterator &nodeIt);

    virtual int count() const override { return fields.size(); }

    virtual bool empty() const override { return fields.empty(); }

    /// Clean out the contents
    void clear() override;
    
    /// Number of fields being represented
    int numFields() const;
    
    /// Returns true if the field exists
    virtual bool hasField(const std::string &name) const override;
    
    /// Returns the field type
    virtual DictionaryType getType(const std::string &name) const override;
    
    /// Remove the given field by name
    void removeField(const std::string &name) override;
    
    /// Return an int, using the default if it's missing
    virtual int getInt(const std::string &name,int defVal) const override;
    virtual int64_t getInt64(const std::string &name,int64_t defVal) const override;
    /// Return a 64 bit unique identity or 0 if missing
    virtual SimpleIdentity getIdentity(const std::string &name) const override;
    /// Interpret an int as a boolean
    virtual bool getBool(const std::string &name,bool defVal) const override;
    /// Interpret an int as a RGBA color
    virtual RGBAColor getColor(const std::string &name,const RGBAColor &defVal) const override;
    /// Return a double, using the default if it's missing
    virtual double getDouble(const std::string &name,double defVal) const override;
    /// Return a string, or empty if it's missing
    virtual std::string getString(const std::string &name) const override;
    /// Return a string, using the default if it's missing
    virtual std::string getString(const std::string &name,const std::string &defVal) const override;
    /// Return an object pointer
    DelayedDeletableRef getObject(const std::string &name);
    /// Return a dictionary as an entry
    virtual DictionaryRef getDict(const std::string &name) const override;
    // Return a generic entry
    virtual DictionaryEntryRef getEntry(const std::string &name) const override;
    // Return an array (if it is an array)
    virtual std::vector<DictionaryEntryRef> getArray(const std::string &name) const override;
    // Return an array of keys
    virtual std::vector<std::string> getKeys() const override;

    /// Set field as int
    void setInt(const std::string &name,int val) override;
    virtual void setInt64(const std::string &name,int64_t val) override;
    /// Set field as 64 bit unique value
    void setIdentifiable(const std::string &name,SimpleIdentity val) override;
    /// Set field as double
    void setDouble(const std::string &name,double val) override;
    /// Set field as string
    void setString(const std::string &name,const std::string &val) override;
    /// Set the dictionary at the given attribute name
    void setDict(const std::string &name,const MutableDictionary_AndroidRef &dict);
    /// Set the entry at the given attribute name
    void setEntry(const std::string &name,const DictionaryEntryRef &entry);
    /// Set the array at the given attribute name
    void setArray(const std::string &name,const std::vector<DictionaryEntryRef> &entries);
    /// Set the array at the given attribute name
    void setArray(const std::string &name,const std::vector<DictionaryRef> &entries);
    /// Set field as pointer
    void setObject(const std::string &name,const DelayedDeletableRef &obj);
    
    // Write data to a raw data buffer
    void asRawData(MutableRawData *rawData);
    
    // Convert to a string for debugging
    std::string toString() const;

    // Merge in key-value pairs from another dictionary
    void addEntries(const Dictionary *other) override;

    // Make a generic ValueRef from a generic entry (yeah, they're different)
    static ValueRef makeValueRef(const DictionaryEntry_AndroidRef &entry);
    static ValueRef makeValueRef(const DictionaryEntryRef &entry);

    struct Value
    {
        virtual ~Value() = default;
        virtual DictionaryType type() const = 0;
        virtual ValueRef copy() const = 0;
        virtual int asInt() const = 0;
        virtual int64_t asInt64() const = 0;
        virtual SimpleIdentity asIdentity() const = 0;
        virtual double asDouble() const = 0;
        virtual void asString(std::string &retStr) const = 0;
        virtual std::string asString() const = 0;
        virtual DelayedDeletableRef asObject() const { return DelayedDeletableRef(); }
        virtual DictionaryRef asDict() const { return DictionaryRef(); }

        virtual bool isEqual(const DictionaryEntry& other) const = 0;
        virtual bool isEqual(const DictionaryEntry_Android& other) const = 0;
    };

    struct StringValue : public Value
    {
        StringValue(const std::string &inVal) : val(inVal) { }
        StringValue(std::string &&inVal) noexcept : val(std::move(inVal)) { }

        virtual DictionaryType type() const override { return DictTypeString; }
        virtual ValueRef copy() const override { return std::make_shared<StringValue>(val); }
        virtual int asInt() const override;
        virtual int64_t asInt64() const override;
        virtual SimpleIdentity asIdentity() const override;
        virtual void asString(std::string &retStr) const override { retStr = val; }
        virtual std::string asString() const override { return val; }
        virtual double asDouble() const override;

        virtual bool isEqual(const DictionaryEntry& other) const override;
        virtual bool isEqual(const DictionaryEntry_Android& other) const override;

        std::string val;
    };

    struct IntValue : public Value
    {
        IntValue(int inVal) : val(inVal) { }

        virtual DictionaryType type() const override { return DictTypeInt; }
        virtual ValueRef copy() const override { return std::make_shared<IntValue>(val); }
        virtual int asInt() const override { return val; }
        virtual int64_t asInt64() const override { return (int64_t)val; }
        virtual SimpleIdentity asIdentity() const override { return val; }
        virtual void asString(std::string &retStr) const override;
        virtual std::string asString() const override;
        virtual double asDouble() const override { return (double)val; }

        virtual bool isEqual(const DictionaryEntry& other) const override { return val == other.getInt(); }
        virtual bool isEqual(const DictionaryEntry_Android& other) const override;

        int val;
    };

    struct Int64Value : public Value
    {
        Int64Value(int64_t inVal) : val(inVal) { }

        virtual DictionaryType type() const override { return DictTypeInt64; }
        virtual ValueRef copy() const override { return std::make_shared<Int64Value>(val); }
        virtual int asInt() const override { return (int)val; }
        virtual int64_t asInt64() const override { return val; }
        virtual SimpleIdentity asIdentity() const override { return (SimpleIdentity)val; }
        virtual void asString(std::string &retStr) const override;
        virtual std::string asString() const override;
        virtual double asDouble() const override { return (double)val; }

        virtual bool isEqual(const DictionaryEntry& other) const override { return val == other.getIdentity(); }
        virtual bool isEqual(const DictionaryEntry_Android& other) const override;

        int64_t val;
    };

    struct DoubleValue : public Value
    {
        DoubleValue(double inVal) : val(inVal) { }

        virtual DictionaryType type() const override { return DictTypeDouble; }
        virtual ValueRef copy() const override { return std::make_shared<DoubleValue>(val); }
        virtual int asInt() const override { return (int)val; }
        virtual int64_t asInt64() const override { return (int64_t)val; }
        virtual SimpleIdentity asIdentity() const override { return EmptyIdentity; }
        virtual void asString(std::string &retStr) const override;
        virtual std::string asString() const override;
        virtual double asDouble() const override { return val; }

        virtual bool isEqual(const DictionaryEntry& other) const override { return val == other.getDouble(); }
        virtual bool isEqual(const DictionaryEntry_Android& other) const override;

        double val;
    };

    struct IdentityValue : public Value
    {
        IdentityValue(SimpleIdentity inVal) : val(inVal) { }

        virtual DictionaryType type() const override { return DictTypeIdentity; }
        virtual ValueRef copy() const override { return std::make_shared<IdentityValue>(val); }
        virtual int asInt() const override { return (int)val; }
        virtual int64_t asInt64() const override { return (int64_t)val; }
        virtual SimpleIdentity asIdentity() const override { return val; }
        virtual void asString(std::string &retStr) const override;
        virtual std::string asString() const override;
        virtual double asDouble() const override { return (double)val; }

        virtual bool isEqual(const DictionaryEntry& other) const override { return val == other.getIdentity(); }
        virtual bool isEqual(const DictionaryEntry_Android& other) const override;

        SimpleIdentity val;
    };

    struct ObjectValue : public Value
    {
        ObjectValue(const DelayedDeletableRef &inVal) : val(inVal) { }

        virtual DictionaryType type() const override { return DictTypeObject; }
        virtual ValueRef copy() const override { return std::make_shared<ObjectValue>(val); }
        virtual int asInt() const override { return 0; }
        virtual int64_t asInt64() const override { return 0; }
        virtual void asString(std::string &retStr) const override { }
        virtual std::string asString() const override { return std::string(); }
        virtual SimpleIdentity asIdentity() const override { return EmptyIdentity; }
        virtual double asDouble() const override { return 0.0; }
        virtual DelayedDeletableRef asObject() const override { return val; }

        virtual bool isEqual(const DictionaryEntry& other) const override { return false; }
        virtual bool isEqual(const DictionaryEntry_Android& other) const override { return false; }

        DelayedDeletableRef val;
    };

    struct DictionaryValue : public Value
    {
        DictionaryValue(const MutableDictionary_AndroidRef &inVal) : val(inVal) { }

        virtual DictionaryType type() const override { return DictTypeDictionary; }
        virtual ValueRef copy() const override { return std::make_shared<DictionaryValue>(val); }
        virtual int asInt() const override { return 0; }
        virtual int64_t asInt64() const override { return 0; }
        virtual void asString(std::string &retStr) const override { }
        virtual std::string asString() const override { return std::string(); }
        virtual SimpleIdentity asIdentity() const override { return EmptyIdentity; }
        virtual double asDouble() const override { return 0.0; }
        virtual DictionaryRef asDict() const override { return val; }

        virtual bool isEqual(const DictionaryEntry& other) const override { return false; }
        virtual bool isEqual(const DictionaryEntry_Android& other) const override { return false; }

        MutableDictionary_AndroidRef val;
    };
    typedef std::shared_ptr<DictionaryValue> DictionaryValueRef;

    struct ArrayValue : public Value
    {
        ArrayValue(ArrayValue &&inVal) noexcept : val(std::move(inVal.val)) { }
        ArrayValue(const std::vector<ValueRef> &inVal) : val(inVal) { }
        ArrayValue(std::vector<ValueRef> &&inVal) noexcept : val(std::move(inVal)) { }
        ArrayValue(const std::vector<DictionaryEntryRef> &inVal);
        ArrayValue(const std::vector<DictionaryRef> &inVal);

        virtual DictionaryType type() const override { return DictTypeArray; }
        virtual ValueRef copy() const override { return std::make_shared<ArrayValue>(val); }
        virtual int asInt() const override { return 0; }
        virtual int64_t asInt64() const override { return 0; }
        virtual void asString(std::string &retStr) const override { }
        virtual std::string asString() const override { return std::string(); }
        virtual SimpleIdentity asIdentity() const override { return EmptyIdentity; }
        virtual double asDouble() const override { return 0.0; }

        virtual bool isEqual(const DictionaryEntry& other) const override { return false; }
        virtual bool isEqual(const DictionaryEntry_Android& other) const override { return false; }

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
    DictionaryEntry_Android()
        : type(DictTypeNone)
    {
    }

    DictionaryEntry_Android(MutableDictionary_Android::ValueRef val)
        : val(val), type(val->type())
    {
    }

    virtual ~DictionaryEntry_Android() = default;

    DictionaryEntry_Android &operator= (const DictionaryEntry_Android &that)
    {
        val = that.val;
        type = that.type;
        return *this;
    }

    /// Returns the field type
    virtual DictionaryType getType() const { return type; }
    /// Return an int, using the default if it's missing
    virtual int getInt() const { return val ? val->asInt() : 0; }
    /// Return an int64, using the default if it's missing
    virtual int64_t getInt64() const { return val ? val->asInt64() : 0; }
    /// Return a 64 bit unique identity or 0 if missing
    virtual SimpleIdentity getIdentity() const { return getInt64(); }
    /// Interpret an int as a boolean
    virtual bool getBool() const { return getInt() != 0; }
    /// Interpret an int as a RGBA color
    virtual RGBAColor getColor() const;
    /// Return a double, using the default if it's missing
    virtual double getDouble() const { return val ? val->asDouble() : 0; }
    /// Return a string, or empty if it's missing
    virtual std::string getString() const { return val ? val->asString() : std::string(); }
    /// Return a dictionary as an entry
    virtual DictionaryRef getDict() const { return val ? val->asDict() : DictionaryRef(); }
    /// Return an array of refs
    virtual std::vector<DictionaryEntryRef> getArray() const;
    /// Compare to other
    virtual bool isEqual(const DictionaryEntryRef &other) const;

    const MutableDictionary_Android::ValueRef &getValue() const { return val; }

protected:
    DictionaryType type;
    MutableDictionary_Android::ValueRef val;
};

}
