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

namespace WhirlyKit
{
    
/// Data types in dictionary
typedef enum {DictTypeNone,DictTypeString,DictTypeInt,DictTypeDouble,DictTypeObject} DictionaryType;

/// The Dictionary is my cross platform replacement for NSDictionary
class Dictionary
{
public:
    Dictionary();
    // Construct from a raw data buffer
    Dictionary(RawData *rawData);
    // Copy constructor
    Dictionary(const Dictionary &that);
    // Assignment operator
    Dictionary &operator = (const Dictionary &that);
    virtual ~Dictionary();
    
    /// Clean out the contents
    void clear();
    
    /// Number of fields being represented
    int numFields() const;
    
    /// Returns true if the field exists
    bool hasField(const std::string &name) const;
    
    /// Returns the field type
    DictionaryType getType(const std::string &name) const;
    
    /// Remove the given field by name
    void removeField(const std::string &name);
    
    /// Return an int, using the default if it's missing
    int getInt(const std::string &name,int defVal=0.0) const;
    /// Interpret an int as a boolean
    bool getBool(const std::string &name,bool defVal=false) const;
    /// Interpret an int as a RGBA color
    RGBAColor getColor(const std::string &name,const RGBAColor &defVal) const;
    /// Return a double, using the default if it's missing
    double getDouble(const std::string &name,double defVal=0.0) const;
    /// Return a string, or empty if it's missing
    std::string getString(const std::string &name) const;
    /// Return a string, using the default if it's missing
    std::string getString(const std::string &name,const std::string &defVal) const;
    /// Return an object pointer
    DelayedDeletableRef getObject(const std::string &name);
    
    /// Set field as int
    void setInt(const std::string &name,int val);
    /// Set field as double
    void setDouble(const std::string &name,double val);
    /// Set field as string
    void setString(const std::string &name,const std::string &val);
    /// Set field as pointer
    void setObject(const std::string &name,DelayedDeletableRef obj);
    
    // Write data to a raw data buffer
    void asRawData(MutableRawData *rawData);
    
protected:
    class Value
    {
    public:
        Value() { }
        virtual ~Value() { }
        
        virtual DictionaryType type() = 0;
        virtual Value *copy() = 0;
        virtual int asInt() = 0;
        virtual void asString(std::string &retStr) = 0;
        virtual double asDouble() = 0;
        virtual DelayedDeletableRef asObject() { return DelayedDeletableRef(); }
    };
    
    class StringValue : public Value
    {
    public:
        StringValue() { }
        StringValue(const std::string &inVal) : val(inVal) { }
        
        virtual DictionaryType type() { return DictTypeString; }
        virtual Value *copy() { return new StringValue(val); }
        virtual int asInt();
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
        virtual Value *copy() { return new IntValue(val); }
        virtual int asInt() { return val; }
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
        virtual Value *copy() { return new DoubleValue(val); }
        virtual int asInt() { return (int)val; }
        virtual void asString(std::string &retStr);
        virtual double asDouble() { return val; }
        
        double val;
    };
    
    class ObjectValue : public Value
    {
    public:
        ObjectValue() { }
        ObjectValue(DelayedDeletableRef inVal) : val(inVal) { }
        
        virtual DictionaryType type() { return DictTypeObject; }
        virtual Value *copy() { return new ObjectValue(val); }
        virtual int asInt() { return 0; }
        virtual void asString(std::string &retStr) { }
        virtual double asDouble() { return 0.0; }
        virtual DelayedDeletableRef asObject() { return val; }
        
        DelayedDeletableRef val;
    };

    typedef std::map<std::string,Value *> FieldMap;
    FieldMap fields;
};
    
}
