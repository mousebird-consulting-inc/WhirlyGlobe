/*
 *  MapboxMultiSourcetileInfo.m
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/23/15.
 *  Copyright 2011-2015 mousebird consulting
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

#import "MapboxMultiSourceTileInfo.h"
#import "MapnikStyleSet.h"
#import "MapboxVectorStyleSet.h"
#import <set>
#import <map>
#import <vector>
#import <string>


// Used to encapsulate a single tile source
// Yes, it's C++.  Suffer.
class SingleTileSource
{
public:
    SingleTileSource() : isImage(true), map(nil), ext(@"png"), styleSet(nil), minZoom(0), maxZoom(22) { }
    
    // Whether it's an image or vector source
    bool isImage;
    // Name of map
    NSString *map;
    NSString *ext;
    // Style sheet, if this is vector
    NSObject<MaplyVectorStyleDelegate> *styleSet;
    MaplyMapnikVectorTileParser *tileParser;
    // Specific tile URLs, if we have them
    NSArray *tileURLs;
    UIColor *backgroundColor;
    int minZoom,maxZoom;
};

@implementation MapboxMultiSourceTileInfo
{
    MaplyBaseViewController *viewC;
    std::vector<NSString *> baseURLs;
    std::vector<SingleTileSource> sources;
    // Sorted by zoom level
    std::vector<std::vector<int> > sourcesByZoom;
    NSMutableDictionary *vecTiles;
}

- (id)initWithViewC:(MaplyBaseViewController *)inViewC
{
    self = [super init];
    if (!self)
        return nil;
    viewC = inViewC;
    self.minZoom = -1;
    self.maxZoom = -1;
    super.pixelsPerSide = 256;
    super.coordSys = [[MaplySphericalMercator alloc] initWebStandard];
    vecTiles = [NSMutableDictionary dictionary];
    
    baseURLs.push_back(@"http://a.tiles.mapbox.com/v4");
    baseURLs.push_back(@"http://b.tiles.mapbox.com/v4");
    
    return self;
}

- (int)minZoom
{
    // Find the first level with an entry
    for (int ii=0;ii<sourcesByZoom.size();ii++)
        if (!sourcesByZoom[ii].empty())
            return ii;
    
    // If this happens, you haven't filled in any data
    return 0;
}

- (int)maxZoom
{
    return sourcesByZoom.size()-1;
}

- (bool)addImageMap:(NSString *)map minZoom:(int)minZoom maxZoom:(int)maxZoom type:(NSString *)imageType
{
    SingleTileSource source;
    source.isImage = true;
    source.map = map;
    source.minZoom = minZoom;  source.maxZoom = maxZoom;
    source.ext = imageType;
    sources.push_back(source);
    [self addedSource:sources.size()-1];
    
    return true;
}

- (bool)addVectorMap:(NSString *)map style:(NSData *)styleData styleType:(MapnikStyleType)styleType minZoom:(int)minZoom maxZoom:(int)maxZoom
{
    SingleTileSource source;
    source.isImage = false;
    source.map = map;
    source.minZoom = minZoom;
    source.maxZoom = maxZoom;
    source.ext = @"vector.pbf";

    // Parse the style sheet
    NSObject<MaplyVectorStyleDelegate> *styleSet = nil;
    switch (styleType)
    {
        case MapnikXMLStyle:
        {
            MapnikStyleSet *mapnikStyleSet = [[MapnikStyleSet alloc] initForViewC:viewC];
            [mapnikStyleSet loadXmlData:styleData];
            [mapnikStyleSet generateStyles];
            source.backgroundColor = mapnikStyleSet.backgroundColor;
            styleSet = mapnikStyleSet;
        }
            break;
        case MapnikMapboxGLStyle:
        {
            MaplyMapboxVectorStyleSet *mapboxStyleSet = [[MaplyMapboxVectorStyleSet alloc] initWithJSON:styleData viewC:viewC];
            styleSet = mapboxStyleSet;
        }
            break;
    }
    
    MaplyMapnikVectorTileParser *tileParser = [[MaplyMapnikVectorTileParser alloc] initWithStyle:styleSet viewC:viewC];
    source.styleSet = styleSet;
    source.tileParser = tileParser;
    sources.push_back(source);
    [self addedSource:sources.size()-1];
    
    return true;
}

- (bool)addTileSpec:(NSDictionary *)jsonDict minZoom:(int)minZoom maxZoom:(int)maxZoom
{
    SingleTileSource source;
    source.isImage = false;
    
    source.tileURLs = jsonDict[@"tiles"];
    if (![source.tileURLs isKindOfClass:[NSArray class]])
        return false;
    NSString *tileURL = source.tileURLs[0];
    if (![tileURL isKindOfClass:[NSString class]])
        return false;
    if ([tileURL containsString:@".png"] || [tileURL containsString:@".jpg"])
        source.isImage = true;
    else if ([tileURL containsString:@".vector.pbf"])
    {
        NSLog(@"Can't handle vector tiles from PBF in MaplyMultiSourceTileInfo");
        return false;
    } else {
        NSLog(@"Don't know what this source is");
        return false;
    }
    
    if (minZoom == -1)
        source.minZoom = [jsonDict[@"minzoom"] intValue];
    else
        source.minZoom = minZoom;
    if (maxZoom == -1)
        source.maxZoom = [jsonDict[@"maxzoom"] intValue];
    else
        source.maxZoom = maxZoom;
    sources.push_back(source);
    [self addedSource:sources.size()-1];
    
    return true;
}

- (bool)addTileSpec:(NSDictionary *)jsonDict
{
    return [self addTileSpec:jsonDict minZoom:-1 maxZoom:-1];
}

- (void)addedSource:(int)which
{
    SingleTileSource &source = sources[which];
    if (source.maxZoom >= sourcesByZoom.size())
        sourcesByZoom.resize(source.maxZoom+1);
    
    for (int zoom = source.minZoom; zoom <= source.maxZoom; zoom++)
    {
        std::vector<int> &inLevel = sourcesByZoom[zoom];
        inLevel.push_back(which);
    }
    
    super.minZoom = [self minZoom];
    super.maxZoom = [self maxZoom];
}

- (void)findParticipatingSources:(std::vector<SingleTileSource *> &)partSources forLevel:(int)level
{
    std::vector<int> &whichIDs = sourcesByZoom[level];
    for (unsigned int ii=0;ii<whichIDs.size();ii++)
    {
        partSources.push_back(&sources[whichIDs[ii]]);
    }
}

// Called by the remote tile source to get a URL to fetch
- (NSURLRequest *)requestForTile:(MaplyTileID)tileID
{
    // Figure out the participating sources
    std::vector<SingleTileSource *> partSources;
    [self findParticipatingSources:partSources forLevel:tileID.level];
    
    if (partSources.size() != 1)
    {
        NSLog(@"Can only deal with one source per level right now.");
        return nil;
    }

    // Pick a base URL and flip the y
    int y = ((int)(1<<tileID.level)-tileID.y)-1;
    
    SingleTileSource *source = partSources[0];
    NSString *fullURLStr = nil;
    if (source->tileURLs)
    {
        NSString *tileURL = source->tileURLs[random()%[source->tileURLs count]];
        fullURLStr = [[[tileURL stringByReplacingOccurrencesOfString:@"{z}" withString:[@(tileID.level) stringValue]]
                                 stringByReplacingOccurrencesOfString:@"{x}" withString:[@(tileID.x) stringValue]]
                                stringByReplacingOccurrencesOfString:@"{y}" withString:[@(y) stringValue]];
    } else {
        // Pick a base URL and build the full URL
        NSString *baseURL = baseURLs[tileID.x%baseURLs.size()];
        fullURLStr = [NSString stringWithFormat:@"%@/%@/%d/%d/%d.%@",baseURL,source->map,tileID.level,tileID.x,y,source->ext];
        if (_accessToken)
            fullURLStr = [fullURLStr stringByAppendingFormat:@"?access_token=%@",_accessToken];
    }
    NSMutableURLRequest *urlReq = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:fullURLStr]];
    if (self.timeOut != 0.0)
        [urlReq setTimeoutInterval:self.timeOut];
        
    return urlReq;
}

#pragma mark - Remote Tile Source Delegate

// Called when the tile has successfully loaded
- (void) remoteTileSource:(MaplyRemoteTileSource *)tileSource tileDidLoad:(MaplyTileID)tileID
{
}

// Called right after the data comes back, so we can mess with it
- (NSData *) remoteTileSource:(MaplyRemoteTileSource *)tileSource modifyTileReturn:(NSData *)tileData forTile:(MaplyTileID)tileID
{
    std::vector<SingleTileSource *> partSources;
    [self findParticipatingSources:partSources forLevel:tileID.level];
    
    if (partSources.empty())
        return tileData;
    
    // Note: Only dealing with one at the moment
    SingleTileSource *source = partSources[0];
    if (source->isImage)
        return tileData;

    if (!_imageLayer)
    {
        NSLog(@"MapboxMultiSourceTileInfo: imageLayer must be set.");
        return tileData;
    }
    
    // Must be a vector tile, so parse it
    MaplyBoundingBox bbox;

    // The tile parser wants bounds in meters(ish)
    [_imageLayer boundsForTile:tileID bbox:&bbox];
    bbox.ll.x *= 20037508.342789244/M_PI;
    bbox.ll.y *= 20037508.342789244/(M_PI);
    bbox.ur.x *= 20037508.342789244/M_PI;
    bbox.ur.y *= 20037508.342789244/(M_PI);
    
    MaplyVectorTileData *vecTileData = [source->tileParser buildObjects:tileData tile:tileID bounds:bbox];
    @synchronized(self)
    {
        vecTiles[MaplyTileIDString(tileID)] = vecTileData;
    }
    
    // Turn on the new data
    [viewC enableObjects:vecTileData.compObjs mode:MaplyThreadCurrent];
    
    // Make up a fake image to return
    CGSize size = CGSizeMake(32, 32);
    UIGraphicsBeginImageContextWithOptions(size, YES, 0);
    [source->backgroundColor setFill];
    UIRectFill(CGRectMake(0, 0, size.width, size.height));
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    return UIImagePNGRepresentation(image);
}

// Letting us know a tile failed to load
- (void) remoteTileSource:(MaplyRemoteTileSource *)tileSource tileDidNotLoad:(MaplyTileID)tileID error:(NSError *)error
{
    MaplyVectorTileData *vecTileData = nil;
    @synchronized(self)
    {
        NSString *ident = MaplyTileIDString(tileID);
        vecTileData  = vecTiles[ident];
        [vecTiles removeObjectForKey:MaplyTileIDString(tileID)];
    }

    if (vecTileData)
        [viewC removeObjects:vecTileData.compObjs];
}

// The given tile unloaded
- (void)remoteTileSource:(MaplyRemoteTileSource *)tileSource tileUnloaded:(MaplyTileID)tileID
{
    MaplyVectorTileData *vecTileData = nil;
    @synchronized(self)
    {
        NSString *ident = MaplyTileIDString(tileID);
        vecTileData  = vecTiles[ident];
        [vecTiles removeObjectForKey:ident];
    }
    
    if (vecTileData)
        [viewC removeObjects:vecTileData.compObjs mode:MaplyThreadCurrent];
}

- (void)remoteTileSource:(id)tileSource tileEnabled:(MaplyTileID)tileID
{
    MaplyVectorTileData *vecTileData = nil;
    @synchronized(self)
    {
        NSString *ident = MaplyTileIDString(tileID);
        vecTileData  = vecTiles[ident];
        [vecTiles removeObjectForKey:ident];
    }
    
    if (vecTileData)
        [viewC enableObjects:vecTileData.compObjs mode:MaplyThreadCurrent];
}

- (void)remoteTileSource:(id)tileSource tileDisabled:(MaplyTileID)tileID
{
    MaplyVectorTileData *vecTileData = nil;
    @synchronized(self)
    {
        NSString *ident = MaplyTileIDString(tileID);
        vecTileData  = vecTiles[ident];
        [vecTiles removeObjectForKey:ident];
    }
    
    if (vecTileData)
        [viewC disableObjects:vecTileData.compObjs mode:MaplyThreadCurrent];
}

@end