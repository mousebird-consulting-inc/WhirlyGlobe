/*
*  MapboxVectorFilter.cpp
*  WhirlyGlobeLib
*
*  Created by Steve Gifford on 4/8/20.
*  Copyright 2011-2020 mousebird consulting
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

#import "MapboxVectorFilter.h"
#import "MapboxVectorStyleSetC.h"
#import "WhirlyKitLog.h"

namespace WhirlyKit
{

MapboxVectorFilter::MapboxVectorFilter()
{
}

static const char * const filterTypes[] = {"==","!=",">",">=","<","<=","in","!in","has","!has","all","any","none"};
static const char * const geomTypes[] = {"Point","LineString","Polygon"};

bool MapboxVectorFilter::parse(const std::vector<DictionaryEntryRef> &filterArray,MapboxVectorStyleSetImpl *styleSet)
{
    if (filterArray.empty()) {
        wkLogLevel(Warn, "Expecting array for filter");
        return false;
    }
    
    geomType = MBGeomNone;
    filterType = (MapboxVectorFilterType)styleSet->enumValue(filterArray[0],filterTypes,MBFilterNone);
    
    if (filterType == MBFilterNone)
    {
        // No filter is easy
    } else if (filterType <= MBFilterLessThanEqual)
    {
        // Filters with two arguments
        if (filterArray.size() < 3)
        {
            wkLogLevel(Warn,"Expecting three arguments for the filter type.");
            return false;
        }

        // Attribute name can be name or geometry type
        attrName = filterArray[1]->getString();
        if (attrName == "$type")
        {
            geomType = (MapboxVectorGeometryType)styleSet->enumValue(filterArray[2], geomTypes, MBGeomNone);
            if (geomType == MBGeomNone)
            {
                wkLogLevel(Warn,"Unrecognized geometry type (%s) in filter",attrName.c_str());
                return false;
            }
        }

        attrVal = filterArray[2];
        if (!attrVal)
            return false;
    } else if (filterType <= MBFilterNotIn) {
        // Filters with inclusion
        std::vector<DictionaryEntryRef> inclVals;
        if (filterArray.size() < 3)
        {
            wkLogLevel(Warn,"Expecting three arugments for the filter type.");
            return false;
        }
        inclVals.reserve(filterArray.size());
        attrName = filterArray[1]->getString();
        for (unsigned int ii=2;ii<filterArray.size();ii++)
        {
            const auto &val = filterArray[ii];
            if (!val)
                return false;
            inclVals.push_back(val);
        }
        attrVals = inclVals;
    } else if (filterType <= MBFilterNotHas)
    {
        // Filters with existence
        if (filterArray.size() < 2)
        {
            wkLogLevel(Warn,"Expecting at least two arguments for filter of type (%s)",filterArray[0]->getString().c_str());
            return false;
        }
        attrName = filterArray[1]->getString();
    } else if (filterType == MBFilterAll || filterType == MBFilterAny)
    {
        // Any and all have subfilters
        for (unsigned int ii=1;ii<filterArray.size();ii++)
        {
            const auto subFilter = std::make_shared<MapboxVectorFilter>();
            if (!subFilter->parse(filterArray[ii]->getArray(), styleSet))
                return false;
            subFilters.push_back(subFilter);
        }
    }
    
    return true;
}

namespace {
    const static std::string geometryType("geometry_type");
}

bool MapboxVectorFilter::testFeature(const Dictionary &attrs,const QuadTreeIdentifier &tileID)
{
    // Compare geometry type
    if (geomType != MBGeomNone)
    {
        const int attrGeomType = attrs.getInt(geometryType) - 1;
        switch (filterType)
        {
            case MBFilterEqual:    return attrGeomType == geomType;
            case MBFilterNotEqual: return attrGeomType != geomType;
            default: break;
        }
    }

    switch (filterType) {
    // Run each of the rules as either AND or OR
    case MBFilterAll:
        for (const auto &filter : subFilters) {
            if (!filter->testFeature(attrs, tileID)) {
                return false;
            }
        }
        return true;
    case MBFilterAny:
        for (const auto &filter : subFilters) {
            if (filter->testFeature(attrs, tileID)) {
                return true;
            }
        }
        return false;
    case MBFilterIn:
    case MBFilterNotIn:
        // Check for attribute value membership
        // Note: Not dealing with differing types well
        if (const auto featAttrVal = attrs.getEntry(attrName)) {
            for (const auto &match : attrVals) {
                if (match->isEqual(featAttrVal)) {
                    return (filterType == MBFilterIn);
                }
            }
        }
        return (filterType != MBFilterIn);
    case MBFilterHas:
        // Check for attribute existence
        return attrs.hasField(attrName);
    case MBFilterNotHas:
        // Check for attribute non-existence
        return !attrs.hasField(attrName);
    default:
        // Equality related operators
        if (auto featAttrVal = attrs.getEntry(attrName)) {
            switch (featAttrVal->getType()) {
            case DictTypeString:
                switch (filterType) {
                    case MBFilterEqual:    return  featAttrVal->isEqual(attrVal);
                    case MBFilterNotEqual: return !featAttrVal->isEqual(attrVal);
                    default: return true;  // Note: Not expecting other comparisons to strings
                }
            case DictTypeInt:
            case DictTypeDouble:
                {
                const double val1 = featAttrVal->getDouble();
                const double val2 = attrVal->getDouble();
                switch (filterType)
                {
                    case MBFilterEqual:            return val1 == val2;
                    case MBFilterNotEqual:         return val1 != val2;
                    case MBFilterGreaterThan:      return val1 > val2;
                    case MBFilterGreaterThanEqual: return val1 >= val2;
                    case MBFilterLessThan:         return val1 < val2;
                    case MBFilterLessThanEqual:    return val1 <= val2;
                    default: return true;
                }
            }
            default:
                wkLogLevel(Warn,"MapboxVectorFilter: Found numeric comparison that doesn't use numbers - '%s', %d/%d/%d",
                           attrName.c_str(), tileID.level, tileID.x, tileID.y);
                return true;
            }
        }
        // No attribute means no pass
        // A missing value and != is valid
        return (filterType == MBFilterNotEqual);
    }
}

}
