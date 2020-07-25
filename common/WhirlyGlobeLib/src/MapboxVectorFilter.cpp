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

static const char *filterTypes[] = {"==","!=",">",">=","<","<=","in","!in","has","!has","all","any","none"};
static const char *geomTypes[] = {"Point","LineString","Polygon"};

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
        attrName = filterArray[1]->getString();
        for (unsigned int ii=2;ii<filterArray.size();ii++)
        {
            DictionaryEntryRef val = filterArray[ii];
            val = filterArray[ii];
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
            MapboxVectorFilterRef subFilter(new MapboxVectorFilter());
            if (!subFilter->parse(filterArray[ii]->getArray(), styleSet))
                return false;
            subFilters.push_back(subFilter);
        }
    }
    
    return true;
}

bool MapboxVectorFilter::testFeature(DictionaryRef attrs,const QuadTreeIdentifier &tileID)
{
    bool ret = true;

    // Compare geometry type
    if (geomType != MBGeomNone)
    {
        int attrGeomType = attrs->getInt("geometry_type") - 1;
        switch (filterType)
        {
            case MBFilterEqual:
                ret = attrGeomType == geomType;
                break;
            case MBFilterNotEqual:
                ret = attrGeomType != geomType;
                break;
            default:
                break;
        }
    } else if (filterType == MBFilterAll || filterType == MBFilterAny)
    {
        // Run each of the rules as either AND or OR
        if (filterType == MBFilterAll)
        {
            for (auto filter : subFilters)
            {
                ret &= filter->testFeature(attrs, tileID);
                if (!ret)
                    break;
            }
        } else if (filterType == MBFilterAny)
        {
            ret = false;
            for (auto filter : subFilters)
            {
                ret |= filter->testFeature(attrs, tileID);
                if (ret)
                    break;
            }
        } else
            ret = false;
    } else if (filterType == MBFilterIn || filterType == MBFilterNotIn)
    {
        // Check for attribute value membership
        bool isIn = false;

        // Note: Not dealing with differing types well
        DictionaryEntryRef featAttrVal = attrs->getEntry(attrName);
        if (featAttrVal)
        {
            for (auto match : attrVals)
            {
                if (match->isEqual(featAttrVal))
                {
                    isIn = true;
                    break;
                }
            }
        }

        ret = (filterType == MBFilterIn ? isIn : !isIn);
    } else if (filterType == MBFilterHas || filterType == MBFilterNotHas)
    {
        // Check for attribute existence
        bool canHas = false;

        DictionaryEntryRef featAttrVal = attrs->getEntry(attrName);
        if (featAttrVal)
            canHas = true;

        ret = (filterType == MBFilterHas ? canHas : !canHas);
    } else {
        // Equality related operators
        DictionaryEntryRef featAttrVal = attrs->getEntry(attrName);
        if (featAttrVal)
        {
            if (featAttrVal->getType() == DictTypeString)
            {
                switch (filterType)
                {
                    case MBFilterEqual:
                        ret = featAttrVal->isEqual(attrVal);
                        break;
                    case MBFilterNotEqual:
                        ret = !featAttrVal->isEqual(attrVal);
                        break;
                    default:
                        // Note: Not expecting other comparisons to strings
                        break;
                }
            } else {
                if (featAttrVal->getType() == DictTypeDouble || featAttrVal->getType() == DictTypeInt)
                {
                    double val1 = featAttrVal->getDouble();
                    double val2 = attrVal->getDouble();
                    switch (filterType)
                    {
                        case MBFilterEqual:
                            ret = val1 == val2;
                            break;
                        case MBFilterNotEqual:
                            ret = val1 != val2;
                            break;
                        case MBFilterGreaterThan:
                            ret = val1 > val2;
                            break;
                        case MBFilterGreaterThanEqual:
                            ret = val1 >= val2;
                            break;
                        case MBFilterLessThan:
                            ret = val1 < val2;
                            break;
                        case MBFilterLessThanEqual:
                            ret = val1 <= val2;
                            break;
                        default:
                            break;
                    }
                } else {
                    wkLogLevel(Warn,"MapboxVectorFilter: Found numeric comparison that doesn't use numbers.");
                }
            }
        } else {
            // No attribute means no pass
            ret = false;

            // A missing value and != is valid
            if (filterType == MBFilterNotEqual)
                ret = true;
        }
    }

    return ret;
}

}
