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

namespace WhirlyKit
{

//@implementation MapboxVectorFilter
//
//- (id)initWithArray:(NSArray *)filterArray styleSet:(MapboxVectorStyleSet *)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
//{
//    if (![filterArray isKindOfClass:[NSArray class]])
//    {
//        NSLog(@"Expecting array for filter");
//        return nil;
//    }
//    if ([filterArray count] < 1)
//    {
//        NSLog(@"Expecting at least one entry in filter");
//        return nil;
//    }
//
//    self = [super init];
//    if (!self)
//        return nil;
//
//    _geomType = MBGeomNone;
//
//    _filterType = (MapboxVectorFilterType)[styleSet enumValue:[filterArray objectAtIndex:0]
//                options:@[@"==",@"!=",@">",@">=",@"<",@"<=",@"in",@"!in",@"has",@"!has",@"all",@"any",@"none"]
//                 defVal:MBFilterNone];
//
//    // Filter with two arguments
//    if (_filterType == MBFilterNone)
//    {
//        // That's easy
//    } else if (_filterType <= MBFilterLessThanEqual)
//    {
//        // Filters with two arguments
//        if ([filterArray count] < 3)
//        {
//            NSLog(@"Expecting three arugments for filter of type (%@)",[filterArray objectAtIndex:0]);
//            return nil;
//        }
//
//        // Attribute name can be name or geometry type
//        _attrName = [filterArray objectAtIndex:1];
//        if ([_attrName isEqualToString:@"$type"])
//        {
//            _geomType = (MapboxVectorGeometryType)[styleSet enumValue:[filterArray objectAtIndex:2] options:@[@"Point",@"LineString",@"Polygon"] defVal:MBGeomNone];
//            if (_geomType == MBGeomNone)
//            {
//                NSLog(@"Unrecognized geometry type (%@) in filter",_attrName);
//                return nil;
//            }
//        }
//
//        _attrVal = [styleSet constantSubstitution:[filterArray objectAtIndex:2] forField:@"Filter attribute value"];
//        if (!_attrVal)
//            return nil;
//    } else if (_filterType <= MBFilterNotIn)
//    {
//        // Filters with inclusion
//        NSMutableArray *inclVals = [NSMutableArray array];
//        if ([filterArray count] < 3)
//        {
//            NSLog(@"Expecting at least three arguments for filter of type (%@)",[filterArray objectAtIndex:0]);
//            return nil;
//        }
//        _attrName = [filterArray objectAtIndex:1];
//        for (unsigned int ii=2;ii<[filterArray count];ii++)
//        {
//            id val = [filterArray objectAtIndex:ii];
//            val = [styleSet constantSubstitution:val forField:@"Filter attribute value"];
//            if (!val)
//                return nil;
//            [inclVals addObject:val];
//        }
//        _attrVals = inclVals;
//    } else if (_filterType <= MBFilterNotHas)
//    {
//        // Filters with existence
//        if ([filterArray count] < 2)
//        {
//            NSLog(@"Expecting at least two arguments for filter of type (%@)",[filterArray objectAtIndex:0]);
//            return nil;
//        }
//        _attrName = [filterArray objectAtIndex:1];
//    } else if (_filterType == MBFilterAll || _filterType == MBFilterAny)
//    {
//        // Any and all have subfilters
//        NSMutableArray *subFilters = [NSMutableArray array];
//        for (unsigned int ii=1;ii<[filterArray count];ii++)
//        {
//            id val = [filterArray objectAtIndex:ii];
//            MapboxVectorFilter *subFilter = [[MapboxVectorFilter alloc] initWithArray:val styleSet:styleSet viewC:viewC];
//            if (!subFilter)
//                return nil;
//            [subFilters addObject:subFilter];
//        }
//        _subFilters = subFilters;
//    }
//
//    return self;
//}
//
//- (bool)testFeature:(NSDictionary *)attrs tile:(MaplyTileID)tileID viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
//{
//    bool ret = true;
//
//    // Compare geometry type
//    if (_geomType != MBGeomNone)
//    {
//        int attrGeomType = [attrs[@"geometry_type"] integerValue] - 1;
//        switch (_filterType)
//        {
//            case MBFilterEqual:
//                ret = attrGeomType == _geomType;
//                break;
//            case MBFilterNotEqual:
//                ret = attrGeomType != _geomType;
//                break;
//            default:
//                break;
//        }
//    } else if (_filterType == MBFilterAll || _filterType == MBFilterAny)
//    {
//        // Run each of the rules as either AND or OR
//        if (_filterType == MBFilterAll)
//        {
//            for (MapboxVectorFilter *filter in _subFilters)
//            {
//                ret &= [filter testFeature:attrs tile:tileID viewC:viewC];
//                if (!ret)
//                    break;
//            }
//        } else if (_filterType == MBFilterAny)
//        {
//            ret = false;
//            for (MapboxVectorFilter *filter in _subFilters)
//            {
//                ret |= [filter testFeature:attrs tile:tileID viewC:viewC];
//                if (ret)
//                    break;
//            }
//        } else
//            ret = false;
//    } else if (_filterType == MBFilterIn || _filterType == MBFilterNotIn)
//    {
//        // Check for attribute value membership
//        bool isIn = false;
//
//        // Note: Not dealing with differing types well
//        id featAttrVal = attrs[_attrName];
//        if (featAttrVal)
//        {
//            for (id match in _attrVals)
//            {
//                if ([match isEqual:featAttrVal])
//                {
//                    isIn = true;
//                    break;
//                }
//            }
//        }
//
//        ret = (_filterType == MBFilterIn ? isIn : !isIn);
//    } else if (_filterType == MBFilterHas || _filterType == MBFilterNotHas)
//    {
//        // Check for attribute existence
//        bool canHas = false;
//
//        id featAttrVal = attrs[_attrName];
//        if (featAttrVal)
//            canHas = true;
//
//        ret = (_filterType == MBFilterHas ? canHas : !canHas);
//    } else {
//        // Equality related operators
//        id featAttrVal = attrs[_attrName];
//        if (featAttrVal)
//        {
//            if ([featAttrVal isKindOfClass:[NSString class]])
//            {
//                switch (_filterType)
//                {
//                    case MBFilterEqual:
//                        ret = [featAttrVal isEqualToString:_attrVal];
//                        break;
//                    case MBFilterNotEqual:
//                        ret = ![featAttrVal isEqualToString:_attrVal];
//                        break;
//                    default:
//                        // Note: Not expecting other comparisons to strings
//                        break;
//                }
//            } else {
//                NSNumber *featAttrNum = (NSNumber *)featAttrVal;
//                NSNumber *attrNum = (NSNumber *)_attrVal;
//                if ([featAttrNum isKindOfClass:[NSNumber class]])
//                {
//                    switch (_filterType)
//                    {
//                        case MBFilterEqual:
//                            ret = [featAttrNum doubleValue] == [attrNum doubleValue];
//                            break;
//                        case MBFilterNotEqual:
//                            ret = [featAttrNum doubleValue] != [attrNum doubleValue];
//                            break;
//                        case MBFilterGreaterThan:
//                            ret = [featAttrNum doubleValue] > [attrNum doubleValue];
//                            break;
//                        case MBFilterGreaterThanEqual:
//                            ret = [featAttrNum doubleValue] >= [attrNum doubleValue];
//                            break;
//                        case MBFilterLessThan:
//                            ret = [featAttrNum doubleValue] < [attrNum doubleValue];
//                            break;
//                        case MBFilterLessThanEqual:
//                            ret = [featAttrNum doubleValue] <= [attrNum doubleValue];
//                            break;
//                        default:
//                            break;
//                    }
//                } else {
//                    NSLog(@"MapboxVectorFilter: Found numeric comparison that doesn't use numbers.");
//                }
//            }
//        } else {
//            // No attribute means no pass
//            ret = false;
//
//            // A missing value and != is valid
//            if (_filterType == MBFilterNotEqual)
//                ret = true;
//        }
//    }
//
//    return ret;
//}
//
//@end

}
