/*
 *  MapboxVectorTileParser.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/25/16.
 *  Copyright 2011-2016 mousebird consulting
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

#import "MapboxVectorTileParser.h"
#import "MaplyVectorStyleC.h"
#import "VectorObject.h"
#import "WhirlyKitLog.h"
#import "DictionaryC.h"
#import "VectorTilePBFParser.h"

#import <vector>
#import <string>

using namespace Eigen;

namespace WhirlyKit
{
    
VectorTileData::VectorTileData()
{
}
    
VectorTileData::VectorTileData(const VectorTileData &that)
    : ident(that.ident), bbox(that.bbox), geoBBox(that.geoBBox)
{
}
    
VectorTileData::~VectorTileData()
{
    for (auto it : vecObjsByStyle)
        delete it.second;
}
    
void VectorTileData::mergeFrom(VectorTileData *that)
{
    compObjs.insert(compObjs.end(),that->compObjs.begin(),that->compObjs.end());
    images.insert(images.end(),that->images.begin(),that->images.end());
    vecObjs.insert(vecObjs.end(),that->vecObjs.begin(),that->vecObjs.end());
    for (auto it : that->vecObjsByStyle) {
        auto it2 = vecObjsByStyle.find(it.first);
        if (it2 != vecObjsByStyle.end())
            it2->second->insert(it2->second->end(),it.second->begin(),it.second->end());
        else
            vecObjsByStyle[it.first] = it.second;
    }
    that->vecObjsByStyle.clear();
    for (auto it : that->categories) {
        categories[it.first] = it.second;
    }
    
    if (!that->changes.empty())
        changes.insert(changes.end(),that->changes.begin(),that->changes.end());
    
    that->clear();
}

void VectorTileData::clear()
{
    compObjs.clear();
    images.clear();
    vecObjs.clear();

    for (auto it : vecObjsByStyle)
        delete it.second;
    vecObjsByStyle.clear();
    categories.clear();
    
    changes.clear();
}

MapboxVectorTileParser::MapboxVectorTileParser(PlatformThreadInfo *inst,VectorStyleDelegateImplRef styleDelegate)
    : localCoords(false), keepVectors(false), parseAll(false), styleDelegate(styleDelegate)
{
    // Index all the categories ahead of time.  Once.
    std::vector<VectorStyleImplRef> allStyles = styleDelegate->allStyles(inst);
    for (VectorStyleImplRef style: allStyles) {
        std::string category = style->getCategory(inst);
        if (!category.empty()) {
            long long styleID = style->getUuid(inst);
            addCategory(category, styleID);
        }
    }
}

MapboxVectorTileParser::~MapboxVectorTileParser()
{
}
    
void MapboxVectorTileParser::setUUIDs(const std::string &name,const std::set<std::string> &uuids)
{
    uuidName = name;
    uuidValues = uuids;
}

void MapboxVectorTileParser::addCategory(const std::string &category,long long styleID)
{
    styleCategories[styleID] = category;
}

static inline double secondsSince(const std::chrono::steady_clock::time_point &t0)
{
    using namespace std::chrono;
    return duration_cast<nanoseconds>(steady_clock::now() - t0).count() / 1.0e9;
}

bool MapboxVectorTileParser::parse(PlatformThreadInfo *styleInst, RawData *rawData, VectorTileData *tileData, volatile bool *cancelBool)
{
    wkLogLevel(Verbose, "MapboxVectorTileParser: Parse [%d/%d/%d] starting",
               tileData->ident.level, tileData->ident.x, tileData->ident.y);
    const auto t0 = std::chrono::steady_clock::now();

    VectorTilePBFParser parser(tileData, &*styleDelegate, styleInst, uuidName, uuidValues,
                               tileData->vecObjsByStyle, localCoords, parseAll,
                               keepVectors ? &tileData->vecObjs : nullptr,
                               [=](){ return cancelBool && *cancelBool; });
    if (!parser.parse(rawData->getRawData(), rawData->getLen()))
    {
        if (parser.getParseCancelled()) {
            const auto duration = secondsSince(t0);
            wkLogLevel(Verbose, "MapboxVectorTileParser: Cancelled [%d/%d/%d] - %.2f MiB - %.4f s",
                       tileData->ident.level, tileData->ident.x, tileData->ident.y,
                       rawData->getLen() / 1024.0 / 1024, duration);
        } else {
            wkLogLevel(Warn, "MapboxVectorTileParser: Parse [%d/%d/%d] failed - '%s'",
                       tileData->ident.level, tileData->ident.x, tileData->ident.y,
                       parser.getErrorString("unknown").c_str());
#if DEBUG
            if (parser.getTotalErrorCount() > 0) {
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

    const auto duration = std::max(1e-9, secondsSince(t0));
    wkLogLevel(Verbose, "MapboxVectorTileParser: Finished [%d/%d/%d] - %.2f MiB - %.4f s - %.4f MiB/s - %.1f features/s",
               tileData->ident.level, tileData->ident.x, tileData->ident.y,
               rawData->getLen() / 1024.0 / 1024,
               duration, rawData->getLen() / duration / 1024 / 1024,
               parser.getFeatureCount() / duration);

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
    for (auto it : tileData->vecObjsByStyle) {
        std::vector<VectorObjectRef> *vecs = it.second;

        auto styleData = std::make_shared<VectorTileData>(*tileData);

        // Ask the subclass to run the style and fill in the VectorTileData
        buildForStyle(styleInst,it.first,*vecs,styleData);
        
        // Sort the results into categories if needed
        auto catIt = styleCategories.find(it.first);
        if (catIt != styleCategories.end() && !styleData->compObjs.empty()) {
            const std::string &category = catIt->second;
            auto compObjs = styleData->compObjs;
            auto categoryIt = tileData->categories.find(category);
            if (categoryIt != tileData->categories.end()) {
                compObjs.insert(compObjs.end(), categoryIt->second.begin(), categoryIt->second.end());
            }
            tileData->categories[category] = compObjs;
        }
        
        // Merge this into the general return data
        tileData->mergeFrom(styleData.get());
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
                                           std::vector<VectorObjectRef> &vecObjs,
                                           VectorTileDataRef data)
{
    VectorStyleImplRef style = styleDelegate->styleForUUID(styleInst,styleID);
    if (style)
        style->buildObjects(styleInst,vecObjs, data);
}
    
}
