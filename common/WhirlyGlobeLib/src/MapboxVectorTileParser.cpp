/*  MapboxVectorTileParser.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/25/16.
 *  Copyright 2011-2022 mousebird consulting
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

#import "MapboxVectorTileParser.h"
#import "MaplyVectorStyleC.h"
#import "VectorObject.h"
#import "WhirlyKitLog.h"
#import "DictionaryC.h"
#import "VectorTilePBFParser.h"

#include <utility>
#import <vector>

using namespace Eigen;

namespace WhirlyKit
{
    
VectorTileData::VectorTileData(const VectorTileData &that)
    : ident(that.ident), bbox(that.bbox), geoBBox(that.geoBBox)
{
}
    
VectorTileData::~VectorTileData()
{
    try
    {
#if defined(DEBUG)
        if (std::any_of(changes.begin(), changes.end(), [](auto i){ return i; }))
        {
            wkLogLevel(Debug, "VectorTileData disposed with pending changes");
        }
#endif
        clear();
    }
    WK_STD_DTOR_CATCH()
}

void VectorTileData::mergeFrom(VectorTileData *that)
{
    compObjs.insert(compObjs.end(),
                    std::make_move_iterator(that->compObjs.begin()),
                    std::make_move_iterator(that->compObjs.end()));
    images.insert(images.end(),
                  std::make_move_iterator(that->images.begin()),
                  std::make_move_iterator(that->images.end()));
    vecObjs.insert(vecObjs.end(),
                   std::make_move_iterator(that->vecObjs.begin()),
                   std::make_move_iterator(that->vecObjs.end()));

    for (auto &kv : that->vecObjsByStyle)
    {
        const auto res = vecObjsByStyle.insert(kv);
        if (!res.second)
        {
            // Already present, merge
            auto &curVal = *(res.first->second);
            curVal.insert(curVal.end(),
                          std::make_move_iterator(kv.second->begin()),
                          std::make_move_iterator(kv.second->end()));
        }
        kv.second = nullptr;    // ownership transferred, don't delete
    }

    for (auto &kv : that->categories)
    {
        const auto res = categories.insert(std::make_pair(kv.first, decltype(kv.second)()));
        auto &newVal = res.first->second;
        if (newVal.empty())
        {
            // replace
            std::swap(newVal, kv.second);
        }
        else
        {
            // merge
            newVal.insert(newVal.end(),
                          std::make_move_iterator(kv.second.begin()),
                          std::make_move_iterator(kv.second.end()));
        }
    }
    
    changes.insert(changes.end(),that->changes.begin(),that->changes.end());
    that->changes.clear();

    that->clear();
}

void VectorTileData::clear()
{
    compObjs.clear();
    images.clear();
    vecObjs.clear();
    categories.clear();

    for (auto it : changes)
    {
        delete it;
    }
    changes.clear();

    for (auto it : vecObjsByStyle)
    {
        delete it.second;
    }
    vecObjsByStyle.clear();
}

MapboxVectorTileParser::MapboxVectorTileParser(PlatformThreadInfo *inst,
                                               VectorStyleDelegateImplRef inStyleDelegate) :
    styleDelegate(std::move(inStyleDelegate))
{
    // Index all the categories ahead of time.  Once.
    if (styleDelegate)
    {
        for (const VectorStyleImplRef &style : styleDelegate->allStyles(inst))
        {
            const std::string category = style->getCategory(inst);
            if (!category.empty())
            {
                const long long styleID = style->getUuid(inst);
                addCategory(category, styleID);
            }
        }
    }
}

void MapboxVectorTileParser::setUUIDName(const std::string &name)
{
    uuidName = name;
}

void MapboxVectorTileParser::setAttributeFilter(const std::string &name,const std::set<std::string> &values)
{
    filterName = name;
    filterValues = values;
}

void MapboxVectorTileParser::setAttributeFilter(const std::string &name,std::set<std::string> &&values)
{
    filterName = name;
    filterValues = std::move(values);
}

void MapboxVectorTileParser::addCategory(const std::string &category,long long styleID)
{
    styleCategories[styleID] = category;
}

static inline double secondsSince(const std::chrono::steady_clock::time_point &t0)
{
    using namespace std::chrono;
    return (double)duration_cast<nanoseconds>(steady_clock::now() - t0).count() / 1.0e9;
}

static bool noCancel(PlatformThreadInfo*) { return false; }

bool MapboxVectorTileParser::parse(PlatformThreadInfo *styleInst, RawData *rawData,
                                   VectorTileData *tileData, volatile bool *cancelBool)
{
    const CancelFunction cancel = [=](auto){ return *cancelBool; };
    return parse(styleInst,rawData,tileData,cancelBool ? cancel : noCancel);
}

bool MapboxVectorTileParser::parse(PlatformThreadInfo *styleInst,
                                   RawData *rawData,
                                   VectorTileData *tileData,
                                   const CancelFunction &cancelFn)
{
//#if DEBUG
//    wkLogLevel(Verbose, "MapboxVectorTileParser: Parse [%d/%d/%d] starting",
//               tileData->ident.level, tileData->ident.x, tileData->ident.y);
//#endif
    const auto t0 = std::chrono::steady_clock::now();

    VectorTilePBFParser parser(tileData, &*styleDelegate, styleInst, filterName, filterValues,
                               tileData->vecObjsByStyle, localCoords, parseAll,
                               keepVectors ? &tileData->vecObjs : nullptr, cancelFn);
    if (!parser.parse(rawData->getRawData(), rawData->getLen()))
    {
        if (parser.getParseCancelled())
        {
            const auto duration = secondsSince(t0);
            wkLogLevel(Verbose, "MapboxVectorTileParser: Cancelled [%d/%d/%d] - %.2f MiB - %.4f s",
                       tileData->ident.level, tileData->ident.x, tileData->ident.y,
                       rawData->getLen() / 1024.0 / 1024, duration);
        }
        else
        {
            wkLogLevel(Warn, "MapboxVectorTileParser: Parse [%d/%d/%d] failed - '%s'",
                       tileData->ident.level, tileData->ident.x, tileData->ident.y,
                       parser.getErrorString("unknown").c_str());
#if DEBUG
            if (parser.getTotalErrorCount() > 0)
            {
                wkLogLevel(Debug,
                           "MapboxVectorTileParser: [%d/%d/%d] parse Errors: %d, Bad Attributes: %d, "
                           "Unknown Commands: %d, Unknown Geom: %d, Unknown Value Types: %d",
                           tileData->ident.level, tileData->ident.x, tileData->ident.y,
                           parser.getParseErrorCount(),
                           parser.getBadAttributeCount(),
                           parser.getUnknownCommandCount(),
                           parser.getUknownGeomTypeCount(),
                           parser.getUnknownValueTypeCount());
            }
#endif
        }
        return false;
    }

#if DEBUG
    const auto duration = std::max(1e-9, secondsSince(t0));
    wkLogLevel(Verbose, "MapboxVectorTileParser: Finished [%d/%d/%d] - %.2f MiB - %.4f s - %.4f MiB/s - %.1f features/s",
               tileData->ident.level, tileData->ident.x, tileData->ident.y,
               rawData->getLen() / 1024.0 / 1024,
               duration, rawData->getLen() / duration / 1024 / 1024,
               parser.getFeatureCount() / duration);
#endif

    // TODO: Switch to stencils and get this working again
    // Call background
//    if (const auto backgroundStyle = styleDelegate->backgroundStyle(styleInst)) {
//        auto styleData = std::make_shared<VectorTileData>(*tileData);
//        std::vector<VectorObjectRef> objs;
//        backgroundStyle->buildObjects(styleInst, objs, styleData);
//
//        tileData->mergeFrom(styleData.get());
//    }
    
    // Run the styles over their assembled data
    for (const auto &it : tileData->vecObjsByStyle)
    {
        std::vector<VectorObjectRef> &vecs = *it.second;

        auto styleData = std::make_shared<VectorTileData>(*tileData);

        // Ask the subclass to run the style and fill in the VectorTileData
        buildForStyle(styleInst,it.first,vecs,styleData,cancelFn);

        // Sort the results into categories if needed
        auto catIt = styleCategories.find(it.first);
        if (catIt != styleCategories.end() && !styleData->compObjs.empty())
        {
            const std::string &category = catIt->second;
            auto &compObjs = styleData->compObjs;
            auto categoryIt = tileData->categories.find(category);
            if (categoryIt != tileData->categories.end())
            {
                compObjs.insert(compObjs.end(), categoryIt->second.begin(), categoryIt->second.end());
            }
            tileData->categories[category] = compObjs;
        }
        
        // Merge this into the general return data
        tileData->mergeFrom(styleData.get());

        // The changes in `tileData` represent objects already tracked
        // in the managers they must be merged or we'll have leaks, so
        // we can't return between the build and the merge above.
        if (cancelFn(styleInst))
        {
            return false;
        }
    }
    
    // These are layered on top for debugging
//    if(debugLabel || debugOutline) {
//        QuadTreeNew::Node tileID = tileData->ident;
//        MbrD geoBounds = tileData->geoBBox;
//        Point2d sw = geoBounds.ll(), ne = geoBounds.ur();
//        if(debugLabel) {
//            MaplyScreenLabel *label = [[MaplyScreenLabel alloc] init];
//            label.text = [NSString stringWithFormat:@"%d: (%d,%d)\n%lu items", tileID.level, tileID.x,
//                          tileID.y, (unsigned long)tileData->compObjs.size()];
//            MaplyCoordinate tileCenter;
//            tileCenter.x = (ne.x() + sw.x())/2.0;
//            tileCenter.y = (ne.y() + sw.y())/2.0;
//            label.loc = tileCenter;
//
//            MaplyComponentObject *c = [viewC addScreenLabels:@[label]
//                                                         desc:@{kMaplyFont : [UIFont boldSystemFontOfSize:12],
//                                                                kMaplyTextColor : [UIColor colorWithRed:0.25 green:0.25 blue:0.25 alpha:0.25],
//                                                                kMaplyDrawPriority : @(kMaplyMaxDrawPriorityDefault+100000000),
//                                                                kMaplyEnable: @(NO)
//                                                                }
//                                                         mode:MaplyThreadCurrent];
//            tileData->compObjs.push_back(c->contents);
//        }
//        if(debugOutline) {
//            MaplyCoordinate outline[5];
//            outline[0].x = ne.x();            outline[0].y = ne.y();
//            outline[1].x = ne.x();            outline[1].y = sw.y();
//            outline[2].x = sw.x();            outline[2].y = sw.y();
//            outline[3].x = sw.x();            outline[3].y = ne.y();
//            outline[4].x = ne.x();            outline[4].y = ne.y();
//            MaplyVectorObject *outlineObj = [[MaplyVectorObject alloc] initWithLineString:outline
//                                                                                numCoords:5
//                                                                               attributes:nil];
//            MaplyComponentObject *c = [viewC addVectors:@[outlineObj]
//                                                    desc:@{kMaplyColor: [UIColor redColor],
//                                                           kMaplyVecWidth:@(4),
//                                                           kMaplyDrawPriority : @(kMaplyMaxDrawPriorityDefault+100000000),
//                                                           kMaplyEnable: @(NO)
//                                                           }
//                                                    mode:MaplyThreadCurrent];
//            tileData->compObjs.push_back(c->contents);
//        }
//    }
    
    return true;
}

void MapboxVectorTileParser::buildForStyle(PlatformThreadInfo *styleInst,
                                           long long styleID,
                                           const std::vector<VectorObjectRef> &vecObjs,
                                           const VectorTileDataRef &data,
                                           const CancelFunction &cancelFn)
    {
        if (auto style = styleDelegate->styleForUUID(styleInst,styleID))
        {
            style->buildObjects(styleInst,vecObjs,data,nullptr,cancelFn);
        }
    }
}
