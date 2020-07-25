/*
*  MapboxVectorFilter.h
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

#import "Dictionary.h"
#import "QuadTreeNew.h"
#import <string>

namespace WhirlyKit
{

/// @brief Mapbox filter operator types
typedef enum {MBFilterEqual,MBFilterNotEqual,MBFilterGreaterThan,MBFilterGreaterThanEqual,MBFilterLessThan,MBFilterLessThanEqual,MBFilterIn,MBFilterNotIn,MBFilterHas,MBFilterNotHas,MBFilterAll,MBFilterAny,MBFilterNone} MapboxVectorFilterType;

/// @brief Mapbox geometry types
typedef enum {MBGeomPoint,MBGeomLineString,MBGeomPolygon,MBGeomNone} MapboxVectorGeometryType;

class MapboxVectorStyleSetImpl;
typedef std::shared_ptr<MapboxVectorStyleSetImpl> MapboxVectorStyleSetImplRef;

class MapboxVectorFilter;
typedef std::shared_ptr<MapboxVectorFilter> MapboxVectorFilterRef;

/// @brief Filter is used to match data in a layer to styles
class MapboxVectorFilter
{
public:
    MapboxVectorFilter();
    
    /// @brief Parse the filter info out of the style entry
    bool parse(const std::vector<DictionaryEntryRef> &styleEntry,MapboxVectorStyleSetImpl *styleSet);

    /// @brief Test a feature's attributes against the filter
    bool testFeature(DictionaryRef attrs,const QuadTreeIdentifier &tileID);

    /// @brief The comparison type for this filter
    MapboxVectorFilterType filterType;

    /// @brief Attribute name for all the types that take two arguments
    std::string attrName;

    /// @brief Set if we're comparing geometry type instead of an attribute
    MapboxVectorGeometryType geomType;

    /// @brief Attribute value to compare for all the type that take two arguments
    DictionaryEntryRef attrVal;

    /// @brief Attribute values for the in and !in operators
    std::vector<DictionaryEntryRef> attrVals;

    /// @brief For All and Any these are the MapboxVectorFilters to evaluate
    std::vector<MapboxVectorFilterRef> subFilters;
};

}
