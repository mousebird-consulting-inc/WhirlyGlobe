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

#import "Dictionary_NSDictionary.h"
#import "NSDictionary+Stuff.h"

namespace WhirlyKit {
    
MutableDictionaryRef MutableDictionaryMake()
{
    return MutableDictionaryRef(new iosMutableDictionary());
}
    
static NSString *StdStringToString(const std::string &str)
{
    return [NSString stringWithFormat:@"%s",str.c_str()];
}
    
static std::string StringToStdString(NSString *str)
{
    if (!str)
        return "";
    return std::string([str cStringUsingEncoding:NSASCIIStringEncoding]);
}
    
iosDictionary::iosDictionary()
{
    dict = [[NSDictionary alloc] init];
}

iosDictionary::iosDictionary(NSDictionary *inDict)
{
    dict = inDict;
}

iosDictionary::iosDictionary(const iosDictionary &that)
{
    dict = [[NSDictionary alloc] initWithDictionary:that.dict];
}

iosDictionary::~iosDictionary()
{
    dict = nil;
}

/// Returns true if the field exists
bool iosDictionary::hasField(const std::string &name) const
{
    return [dict objectForKey:StdStringToString(name)] != nil;
}

/// Returns the field type
DictionaryType iosDictionary::getType(const std::string &name) const
{
    id obj = [dict objectForKey:StdStringToString(name)];
    if (obj == nil)
        return DictTypeNone;
    
    if ([obj isKindOfClass:[NSString class]])
        return DictTypeString;
    else if ([obj isKindOfClass:[NSNumber class]]) {
        // Note: Can't tell the difference between the types
        return DictTypeDouble;
    }
    
    return DictTypeObject;
}

/// Return an int, using the default if it's missing
int iosDictionary::getInt(const std::string &name,int defVal) const
{
    NSString *theName = StdStringToString(name);
    
    return [dict intForKey:theName default:defVal];
}

/// Return a 64 bit unique identity or 0 if missing
SimpleIdentity iosDictionary::getIdentity(const std::string &name) const
{
    NSString *theName = StdStringToString(name);

    return [dict intForKey:theName default:EmptyIdentity];
}

/// Interpret an int as a boolean
bool iosDictionary::getBool(const std::string &name,bool defVal) const
{
    NSString *theName = StdStringToString(name);

    return [dict boolForKey:theName default:defVal];
}

/// Interpret an int as a RGBA color
RGBAColor iosDictionary::getColor(const std::string &name,const RGBAColor &defVal) const
{
//    NSString *theName = StdStringToString(name);

    throw "Not implemented";
    return RGBAColor();
}

/// Return a double, using the default if it's missing
double iosDictionary::getDouble(const std::string &name,double defVal) const
{
    NSString *theName = StdStringToString(name);

    return [dict doubleForKey:theName default:defVal];
}

/// Return a string, or empty if it's missing
std::string iosDictionary::getString(const std::string &name) const
{
    NSString *theName = StdStringToString(name);

    return StringToStdString([dict stringForKey:theName default:nil]);
}

/// Return a string, using the default if it's missing
std::string iosDictionary::getString(const std::string &name,const std::string &defVal) const
{
    NSString *theName = StdStringToString(name);

    return StringToStdString([dict stringForKey:theName default:StdStringToString(defVal)]);
}


iosMutableDictionary::iosMutableDictionary()
{
    dict = [[NSMutableDictionary alloc] init];
}

iosMutableDictionary::iosMutableDictionary(NSMutableDictionary *inDict)
{
    dict = inDict;
}
    
iosMutableDictionary::iosMutableDictionary(MutableDictionaryRef inDict)
{
    iosMutableDictionary *other = dynamic_cast<iosMutableDictionary *>(inDict.get());
    if (other)
        dict = [[NSMutableDictionary alloc] initWithDictionary:other->dict];
}

// Assignment operator
iosMutableDictionary & iosMutableDictionary::operator = (const iosMutableDictionary &that)
{
    dict = [[NSMutableDictionary alloc] initWithDictionary:that.dict];
    
    return *this;
}

iosMutableDictionary::~iosMutableDictionary()
{
    dict = nil;
}
    
/// Returns true if the field exists
bool iosMutableDictionary::hasField(const std::string &name) const
{
    return [dict objectForKey:StdStringToString(name)] != nil;
}

/// Returns the field type
DictionaryType iosMutableDictionary::getType(const std::string &name) const
{
    id obj = [dict objectForKey:StdStringToString(name)];
    if (obj == nil)
        return DictTypeNone;
    
    if ([obj isKindOfClass:[NSString class]])
        return DictTypeString;
    else if ([obj isKindOfClass:[NSNumber class]]) {
        // Note: Can't tell the difference between the types
        return DictTypeDouble;
    }
    
    return DictTypeObject;
}

/// Return an int, using the default if it's missing
int iosMutableDictionary::getInt(const std::string &name,int defVal) const
{
    NSString *theName = StdStringToString(name);
    
    return [dict intForKey:theName default:defVal];
}

/// Return a 64 bit unique identity or 0 if missing
SimpleIdentity iosMutableDictionary::getIdentity(const std::string &name) const
{
    NSString *theName = StdStringToString(name);
    
    return [dict intForKey:theName default:EmptyIdentity];
}

/// Interpret an int as a boolean
bool iosMutableDictionary::getBool(const std::string &name,bool defVal) const
{
    NSString *theName = StdStringToString(name);
    
    return [dict boolForKey:theName default:defVal];
}

/// Interpret an int as a RGBA color
RGBAColor iosMutableDictionary::getColor(const std::string &name,const RGBAColor &defVal) const
{
    //    NSString *theName = StdStringToString(name);
    
    throw "Not implemented";
    return RGBAColor();
}

/// Return a double, using the default if it's missing
double iosMutableDictionary::getDouble(const std::string &name,double defVal) const
{
    NSString *theName = StdStringToString(name);
    
    return [dict doubleForKey:theName default:defVal];
}

/// Return a string, or empty if it's missing
std::string iosMutableDictionary::getString(const std::string &name) const
{
    NSString *theName = StdStringToString(name);
    
    return StringToStdString([dict stringForKey:theName default:nil]);
}

/// Return a string, using the default if it's missing
std::string iosMutableDictionary::getString(const std::string &name,const std::string &defVal) const
{
    NSString *theName = StdStringToString(name);
    
    return StringToStdString([dict stringForKey:theName default:StdStringToString(defVal)]);
}
    
void iosMutableDictionary::clear()
{
    dict = [[NSMutableDictionary alloc] init];
}

/// Remove the given field by name
void iosMutableDictionary::removeField(const std::string &name)
{
    [dict removeObjectForKey:StdStringToString(name)];
}
    
/// Set field as int
void iosMutableDictionary::setInt(const std::string &name,int val)
{
    dict[StdStringToString(name)] = @(val);
}

/// Set field as 64 bit unique value
void iosMutableDictionary::setIdentifiable(const std::string &name,SimpleIdentity val)
{
    dict[StdStringToString(name)] = @(val);
}

/// Set field as double
void iosMutableDictionary::setDouble(const std::string &name,double val)
{
    dict[StdStringToString(name)] = @(val);
}

/// Set field as string
void iosMutableDictionary::setString(const std::string &name,const std::string &val)
{
    dict[StdStringToString(name)] = StdStringToString(val);
}

// Merge in key-value pairs from another dictionary
void iosMutableDictionary::addEntries(const Dictionary *other)
{
    const iosDictionary *iosDict = dynamic_cast<const iosDictionary *>(other);
    if (iosDict) {
        [dict addEntriesFromDictionary:iosDict->dict];
    } else {
        const iosMutableDictionary *iosMutDict = dynamic_cast<const iosMutableDictionary *>(other);
        if (iosMutDict) {
            [dict addEntriesFromDictionary:iosMutDict->dict];
        }
    }
}

}
