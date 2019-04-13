/*
 *  MapboxVectorTileParser_iOS.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/10/19.
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

#import "MapboxVectorTileParser_iOS.h"
#import "Dictionary_NSDictionary.h"
#import "MaplyVectorObject_private.h"
#import "MapboxVectorTiles_private.h"
#import "MaplyScreenLabel.h"
#import "MaplyComponentObject_private.h"

namespace WhirlyKit
{

MapboxVectorTileParser_iOS::MapboxVectorTileParser_iOS(NSObject<MaplyVectorStyleDelegate> * styleDelegate,NSObject<MaplyRenderControllerProtocol> *viewC)
    : styleDelegate(styleDelegate), viewC(viewC), debugLabel(false), debugOutline(false)
{
    // Index all the categories ahead of time.  Once.
    NSArray *allStyles = [styleDelegate allStyles];
    for (NSObject<MaplyVectorStyle> *style in allStyles) {
        NSString *category = [style getCategory];
        if (category) {
            long long styleID = style.uuid;
            std::string categoryStr = [category cStringUsingEncoding:NSUTF8StringEncoding];
            addCategory(categoryStr, styleID);
        }
    }
}

MapboxVectorTileParser_iOS::~MapboxVectorTileParser_iOS()
{
}
    
bool MapboxVectorTileParser_iOS::layerShouldParse(const std::string &layerName,VectorTileData *tileData)
{
    NSString *layerNameStr = [NSString stringWithUTF8String:layerName.c_str()];
    MaplyTileID tileID;
    tileID.x = tileData->ident.x;  tileID.y = tileData->ident.y;  tileID.level = tileData->ident.level;

    return [styleDelegate layerShouldDisplay:layerNameStr tile:tileID];
}

// Return a set of styles that will parse the given feature
SimpleIDSet MapboxVectorTileParser_iOS::stylesForFeature(MutableDictionaryRef attributes,const std::string &layerName,VectorTileData *tileData)
{
    iosMutableDictionaryRef dict = std::dynamic_pointer_cast<iosMutableDictionary>(attributes);
    NSString *layerNameStr = [NSString stringWithUTF8String:layerName.c_str()];
    MaplyTileID tileID;
    tileID.x = tileData->ident.x;  tileID.y = tileData->ident.y;  tileID.level = tileData->ident.level;

    NSArray *styles = [styleDelegate stylesForFeatureWithAttributes:dict->dict onTile:tileID inLayer:layerNameStr viewC:viewC];
    SimpleIDSet styleIDs;
    for (NSObject<MaplyVectorStyle> *style in styles) {
        styleIDs.insert(style.uuid);
    }
    
    return styleIDs;
}
    
void MapboxVectorTileParser_iOS::buildForStyle(long long styleID,std::vector<VectorObjectRef> &vecs,VectorTileDataRef data)
{
    // Make up an NSArray for this since it's outward facing
    NSMutableArray *vecObjs = [[NSMutableArray alloc] init];
    for (auto vec : vecs) {
        MaplyVectorObject *wrapObj = [[MaplyVectorObject alloc] initWithRef:vec];
        [vecObjs addObject:wrapObj];
    }
    
    NSObject<MaplyVectorStyle> *style = [styleDelegate styleForUUID:styleID viewC:viewC];
    if (!style)
        return;
    MaplyVectorTileData *tileDataWrap = [[MaplyVectorTileData alloc] initWithTileData:data];
    [style buildObjects:vecObjs forTile:tileDataWrap viewC:viewC];
}
    
bool MapboxVectorTileParser_iOS::parse(RawData *rawData,VectorTileData *tileData)
{
    // This one does the real work
    if (!MapboxVectorTileParser::parse(rawData,tileData))
        return false;

    // These are layered on top for debugging
    if(debugLabel || debugOutline) {
        QuadTreeNew::Node tileID = tileData->ident;
        MbrD geoBounds = tileData->geoBBox;
        Point2d sw = geoBounds.ll(), ne = geoBounds.ur();
        if(debugLabel) {
            MaplyScreenLabel *label = [[MaplyScreenLabel alloc] init];
            label.text = [NSString stringWithFormat:@"%d: (%d,%d)\n%lu items", tileID.level, tileID.x,
                          tileID.y, (unsigned long)tileData->compObjs.size()];
            MaplyCoordinate tileCenter;
            tileCenter.x = (ne.x() + sw.x())/2.0;
            tileCenter.y = (ne.y() + sw.y())/2.0;
            label.loc = tileCenter;
            
            MaplyComponentObject *c = [viewC addScreenLabels:@[label]
                                                         desc:@{kMaplyFont : [UIFont boldSystemFontOfSize:12],
                                                                kMaplyTextColor : [UIColor colorWithRed:0.25 green:0.25 blue:0.25 alpha:0.25],
                                                                kMaplyDrawPriority : @(kMaplyMaxDrawPriorityDefault+100000000),
                                                                kMaplyEnable: @(NO)
                                                                }
                                                         mode:MaplyThreadCurrent];
            tileData->compObjs.push_back(c->contents);
        }
        if(debugOutline) {
            MaplyCoordinate outline[5];
            outline[0].x = ne.x();            outline[0].y = ne.y();
            outline[1].x = ne.x();            outline[1].y = sw.y();
            outline[2].x = sw.x();            outline[2].y = sw.y();
            outline[3].x = sw.x();            outline[3].y = ne.y();
            outline[4].x = ne.x();            outline[4].y = ne.y();
            MaplyVectorObject *outlineObj = [[MaplyVectorObject alloc] initWithLineString:outline
                                                                                numCoords:5
                                                                               attributes:nil];
            MaplyComponentObject *c = [viewC addVectors:@[outlineObj]
                                                    desc:@{kMaplyColor: [UIColor redColor],
                                                           kMaplyVecWidth:@(4),
                                                           kMaplyDrawPriority : @(kMaplyMaxDrawPriorityDefault+100000000),
                                                           kMaplyEnable: @(NO)
                                                           }
                                                    mode:MaplyThreadCurrent];
            tileData->compObjs.push_back(c->contents);
        }
    }
    
    return true;
}
    
}
