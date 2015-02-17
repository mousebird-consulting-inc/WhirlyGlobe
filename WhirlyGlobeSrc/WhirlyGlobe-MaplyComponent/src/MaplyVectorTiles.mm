/*
 *  MaplyVectorTiles.m
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/3/14.
 *  Copyright 2011-2014 mousebird consulting
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

#import "MaplyVectorTiles.h"
#import "FMDatabase.h"
#import "FMDatabaseQueue.h"
#import "NSData+Zlib.h"
#import "MaplyVectorObject_private.h"
#import "MaplyScreenLabel.h"
#import "MaplyIconManager.h"
#import "MaplyVectorLineStyle.h"
#import "MaplyVectorMarkerStyle.h"
#import "MaplyVectorPolygonStyle.h"
#import "MaplyVectorTextStyle.h"
#import "AFHTTPRequestOperation.h"
#import <string>
#import <map>
#import <vector>

using namespace Maply;

typedef std::map<std::string,MaplyVectorTileStyle *> StyleMap;

@interface MaplyVectorTiles()
@property (nonatomic,readonly) NSString *tilesDir;
@end

@implementation MaplyVectorTiles
{
    // If we're reading from a database, these are set
    FMDatabase *db;
    FMDatabaseQueue *queue;
    bool compressed;
    
    // The style info
    StyleMap styleObjects;
    
    // If we're fetching remotely, the tile URLs
    NSArray *tileURLs;
}

+ ( void)StartRemoteVectorTiles:(NSString *)jsonURL cacheDir:(NSString *)cacheDir viewC:(MaplyBaseViewController *)viewC block:(void (^)(MaplyVectorTiles *vecTiles))callbackBlock
{
    // First we need the JSON describing the whole thing
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:jsonURL]];
    request.cachePolicy = NSURLRequestReloadIgnoringLocalCacheData;
    AFHTTPRequestOperation *operation = [[AFHTTPRequestOperation alloc] initWithRequest:request];
    operation.responseSerializer = [AFJSONResponseSerializer serializer];
    [operation setCompletionBlockWithSuccess:^(AFHTTPRequestOperation *operation, id responseObject) {
        NSDictionary *jsonDict = responseObject;
        // We're expecting one or more tile URLs and the styles
        NSString *styleURL = jsonDict[@"style"];
        NSArray *tileURLs = jsonDict[@"tiles"];
        if (![styleURL isKindOfClass:[NSString class]])
        {
            NSLog(@"Expecting style URL in vector tile spec from: %@",jsonURL);
        } else if (![tileURLs isKindOfClass:[NSArray class]] || [tileURLs count] == 0)
        {
            NSLog(@"Expecting one or more tile URLs in vector tile spec from: %@",jsonURL);
        } else {
            // Success, go get the styles
            NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:styleURL]];
            request.cachePolicy = NSURLRequestReloadIgnoringLocalCacheData;
            AFHTTPRequestOperation *operation = [[AFHTTPRequestOperation alloc] initWithRequest:request];
            operation.responseSerializer = [AFJSONResponseSerializer serializer];
            [operation setCompletionBlockWithSuccess:^(AFHTTPRequestOperation *operation, id responseObject) {
                // This should be enough to start, so let's do that
                NSDictionary *styleDict = responseObject;
                MaplyVectorTiles *vecTiles = [[MaplyVectorTiles alloc] initWithTileSpec:jsonDict styles:styleDict viewC:viewC];
                vecTiles.cacheDir = cacheDir;
                if (vecTiles)
                {
                    // Up to the "caller" to do something with it
                    dispatch_async(dispatch_get_main_queue(),
                                   ^{
                                       callbackBlock(vecTiles);
                                   });
                } else {
                    NSLog(@"Unable to create MaplyVectorTiles from URL: %@",jsonURL);
                    dispatch_async(dispatch_get_main_queue(),
                                   ^{
                                       callbackBlock(nil);
                                   });
                }
            } failure:^(AFHTTPRequestOperation *operation, NSError *error) {
                NSLog(@"Failed to reach vector styles at: %@",styleURL);
                dispatch_async(dispatch_get_main_queue(),
                               ^{
                                   callbackBlock(nil);
                               });
            }];
            [operation start];
        }
    } failure:^(AFHTTPRequestOperation *operation, NSError *error) {
        NSLog(@"Failed to reach JSON vector tile spec at: %@",jsonURL);
        dispatch_async(dispatch_get_main_queue(),
                       ^{
                           callbackBlock(nil);
                       });
    }];
    [operation start];
}

// Parse a UIColor from hex values
+ (UIColor *) ParseColor:(NSString *)colorStr
{
  return [MaplyVectorTiles ParseColor:colorStr alpha:1.0];
}

+ (UIColor *) ParseColor:(NSString *)colorStr alpha:(CGFloat)alpha
{
    // Hex color string
    if ([colorStr characterAtIndex:0] == '#')
    {
      int red = 255, green = 255, blue = 255;
      // parse the hex
      NSScanner *scanner = [NSScanner scannerWithString:colorStr];
      unsigned int colorVal;
      [scanner setScanLocation:1]; // bypass #
      [scanner scanHexInt:&colorVal];
      blue = colorVal & 0xFF;
      green = (colorVal >> 8) & 0xFF;
      red = (colorVal >> 16) & 0xFF;
      
      return [UIColor colorWithRed:red/255.0*alpha green:green/255.0*alpha blue:blue/255.0*alpha alpha:alpha];
    } else if ([colorStr rangeOfString:@"rgba"].location == 0)
    {
        NSScanner *scanner = [NSScanner scannerWithString:colorStr];
        NSMutableCharacterSet *skipSet = [[NSMutableCharacterSet alloc] init];
        [skipSet addCharactersInString:@"(), "];
        [scanner setCharactersToBeSkipped:skipSet];
        [scanner setScanLocation:5];
        int red,green,blue;
        [scanner scanInt:&red];
        [scanner scanInt:&green];
        [scanner scanInt:&blue];
        float locAlpha;
        [scanner scanFloat:&locAlpha];
        
        return [UIColor colorWithRed:red/255.0*alpha green:green/255.0*alpha blue:blue/255.0*alpha alpha:locAlpha*alpha];
    }
    
    return [UIColor colorWithWhite:1.0 alpha:alpha];
}

- (id)initWithDirectory:(NSString *)tilesDir viewC:(MaplyBaseViewController *)viewC
{
    self = [super init];
    _viewC = viewC;
    _tilesDir = tilesDir;
    
    // Look for the styles file
    NSData *styleData = [NSData dataWithContentsOfFile:[NSString stringWithFormat:@"%@/styles.json",tilesDir]];
    if (!styleData)
        return nil;
    
    // Convert to a dictionary
    NSError *error = nil;
    NSDictionary *styleDict = [NSJSONSerialization JSONObjectWithData:styleData options:NULL error:&error];
    if (error || ![styleDict isKindOfClass:[NSDictionary class]])
        return nil;
    
    NSDictionary *paramDict = styleDict[@"parameters"];
    if (![paramDict isKindOfClass:[NSDictionary class]])
        return nil;
    _minLevel = (int)[paramDict[@"minLevel"] integerValue];
    _maxLevel = (int)[paramDict[@"maxLevel"] integerValue];
    
    NSArray *layers = styleDict[@"layers"];
    if (![layers isKindOfClass:[NSArray class]])
        return nil;
    _layerNames = layers;
    
    NSArray *styles = styleDict[@"styles"];
    if (![styles isKindOfClass:[NSArray class]])
        return nil;
    _styles = styles;
    
    _settings = [[MaplyVectorTileStyleSettings alloc] init];
    _settings.lineScale = [UIScreen mainScreen].scale;
    _settings.textScale = [UIScreen mainScreen].scale;
    _settings.markerScale = [UIScreen mainScreen].scale;
    
    return self;
}

- (id)initWithDatabase:(NSString *)name viewC:(MaplyBaseViewController *)viewC
{
    self = [super init];
    _viewC = viewC;
    NSString *infoPath = nil;
    // See if that was a direct path first
    if ([[NSFileManager defaultManager] fileExistsAtPath:name])
        infoPath = name;
    else {
        // Now try looking for it in the bundle
        infoPath = [[NSBundle mainBundle] pathForResource:name ofType:@"sqlite"];
        if (!infoPath)
            return nil;
    }
    
    db = [[FMDatabase alloc] initWithPath:infoPath];
    if (!db)
        return nil;
    
    [db openWithFlags:SQLITE_OPEN_READONLY];
    queue = [FMDatabaseQueue databaseQueueWithPath:infoPath];
    
    // Basic info about the database.  Ignoring extents for now
    FMResultSet *res = [db executeQuery:@"SELECT minlevel,maxlevel,compressed FROM manifest"];
    if ([res next])
    {
        _minLevel = [res intForColumn:@"minlevel"];
        _maxLevel = [res intForColumn:@"maxlevel"];
        compressed = [res boolForColumn:@"compressed"];
    }
    
    // Layer names
    res = [db executeQuery:@"SELECT name FROM layers"];
    NSMutableArray *layerNames = [NSMutableArray array];
    while ([res next])
    {
        NSString *layerName = [res stringForColumn:@"name"];
        [layerNames addObject:layerName];
    }
    _layerNames = layerNames;
    if ([layerNames count] == 0)
        return nil;
    
    // Style entries.  Each is a little bit of JSON
    res = [db executeQuery:@"SELECT name,style FROM styles"];
    NSMutableArray *styles = [NSMutableArray array];
    while ([res next])
    {
//        NSString *styleName = [res stringForColumn:@"name"];
        NSData *styleInfo = [res dataForColumn:@"style"];
        // Convert to an NSDictionary
        NSError *error = nil;
        NSDictionary *styleDict = [NSJSONSerialization JSONObjectWithData:styleInfo options:NULL error:&error];
        if (error || ![styleDict isKindOfClass:[NSDictionary class]])
            return nil;
        [styles addObject:styleDict];
    }
    _styles = styles;
    
    _settings = [[MaplyVectorTileStyleSettings alloc] init];
    _settings.lineScale = [UIScreen mainScreen].scale;
    _settings.textScale = [UIScreen mainScreen].scale;
    _settings.markerScale = [UIScreen mainScreen].scale;
    _settings.mapScaleScale = 1.0;
    
    return self;
}

- (id)initWithTileSpec:(NSDictionary *)jsonSpec styles:(NSDictionary *)styleDict viewC:(MaplyBaseViewController *)viewC
{
    self = [super init];
    _viewC = viewC;
    
    tileURLs = jsonSpec[@"tiles"];
    if (![tileURLs isKindOfClass:[NSArray class]])
        return nil;
    
    NSDictionary *paramDict = styleDict[@"parameters"];
    if (![paramDict isKindOfClass:[NSDictionary class]])
        return nil;
    // Note: Pull these out of the tile JSON
    _minLevel = (int)[paramDict[@"minLevel"] integerValue];
    _maxLevel = (int)[paramDict[@"maxLevel"] integerValue];
    
    NSArray *layers = styleDict[@"layers"];
    if (![layers isKindOfClass:[NSArray class]])
        return nil;
    _layerNames = layers;
    
    NSArray *styles = styleDict[@"styles"];
    if (![styles isKindOfClass:[NSArray class]])
        return nil;
    _styles = styles;
    
    _settings = [[MaplyVectorTileStyleSettings alloc] init];
    _settings.lineScale = [UIScreen mainScreen].scale;
    _settings.textScale = [UIScreen mainScreen].scale;
    _settings.markerScale = [UIScreen mainScreen].scale;
    
    return self;
}

- (void)setTilesDB:(NSString *)tilesDB
{
    // Note: fill this in
}

- (int)minZoom
{
    return _minLevel;
}

- (int)maxZoom
{
    return _maxLevel;
}

- (void)setCacheDir:(NSString *)cacheDir
{
    _cacheDir = cacheDir;
    // Make sure it exists
    NSError *error = nil;
    if (![[NSFileManager defaultManager] fileExistsAtPath:cacheDir])
        [[NSFileManager defaultManager] createDirectoryAtPath:cacheDir withIntermediateDirectories:YES attributes:nil error:&error];
}

// Return or create the object which will create the given style
- (MaplyVectorTileStyle *)getStyle:(const std::string &)uuid
{
    // Note: Is this too much locking?
    @synchronized(self)
    {
        StyleMap::iterator it = styleObjects.find(uuid);
        MaplyVectorTileStyle *styleObj = nil;
        if (it == styleObjects.end())
        {
            // Look for the corresponding style
            // Note: index this
            NSDictionary *theStyle = nil;
            for (NSDictionary *style in _styles)
            {
                NSString *uuidStr = style[@"uuid"];
                std::string thisUuid = [uuidStr cStringUsingEncoding:NSASCIIStringEncoding];
                if (!uuid.compare(thisUuid))
                {
                    theStyle = style;
                    break;
                }
            }
            if (!theStyle)
                return nil;
            NSMutableDictionary *thisStyleDict = [NSMutableDictionary dictionaryWithDictionary:theStyle];
            thisStyleDict[kMaplySelectable] = @(_selectable);
            styleObj = [MaplyVectorTileStyle styleFromStyleEntry:thisStyleDict settings:_settings viewC:_viewC];
            styleObjects[uuid] = styleObj;
        } else
            styleObj = it->second;
        
        return styleObj;
    }
}

// Count the number of each type of object (debugging)
//self countVecs:vecObj points:&numPoints linears:&numLinears areals:&numAreals
- (void)countVecs:(MaplyVectorObject *)vecObj points:(int *)numPts linears:(int *)numLins areals:(int *)numArs
{
    NSArray *vecs = [vecObj splitVectors];
    for (MaplyVectorObject *vec in vecs)
    {
        switch (vec.vectorType)
        {
            case MaplyVectorPointType:
                (*numPts)++;
                break;
            case MaplyVectorLinearType:
                (*numLins)++;
                break;
            case MaplyVectorArealType:
                (*numArs)++;
                break;
            default:
                break;
        }
    }
}

// Fetch a given tile, either from the file system or from the database
- (MaplyVectorObject *)readTile:(MaplyTileID)tileID layer:(NSString *)layerName
{
    MaplyVectorObject *vecObj = nil;
    
    if (db)
    {
        // Put together the precalculated quad index.  This is faster
        //  than x,y,level
        int quadIdx = 0;
        for (int iq=0;iq<tileID.level;iq++)
            quadIdx += (1<<iq)*(1<<iq);
        quadIdx += tileID.y*(1<<tileID.level)+tileID.x;
        
        NSData * __block uncompressedData=nil;
        bool __block tilePresent = false;
        // Note: Need to sort this out
        [queue inDatabase:^(FMDatabase *theDb) {
            // Now look for the tile
            FMResultSet *res = [theDb executeQuery:[NSString stringWithFormat:@"SELECT data FROM %@_table WHERE quadindex=%d;",layerName,quadIdx]];
            NSData *data = nil;
            if ([res next])
            {
                tilePresent = true;
                data = [res dataForColumn:@"data"];
            }
            // Note: Can we move this out of the block?
            if (compressed)
            {
                if (data && [data length] > 0)
                    uncompressedData = [data uncompressGZip];
            } else
                uncompressedData = data;
            [res close];
        }];

        // Turn the raw data into vectors
        if (tilePresent && uncompressedData)
        {
            vecObj = [MaplyVectorObject VectorObjectFromVectorDBRaw:uncompressedData];
        }
    } else {
        NSString *fileName = [NSString stringWithFormat:@"%@/%d/%d/%d%@",_tilesDir,tileID.level,tileID.y,tileID.x,layerName];
        vecObj = [MaplyVectorObject VectorObjectFromShapeFile:fileName];
    }

//    if (vecObj)
//    {
//        int numPoints=0, numLinears=0, numAreals=0;
//        [self countVecs:vecObj points:&numPoints linears:&numLinears areals:&numAreals];
//        NSLog(@"Loading tile: %@ %d: (%d,%d)\n\tpoints = %d, linears = %d, areals = %d",layerName,tileID.level,tileID.x,tileID.y,numPoints,numLinears,numAreals);
//    }

    return vecObj;
}

typedef std::map<std::string,NSMutableArray *> VecsForStyles;

// Fetch all the layers at once
- (NSArray *)fetchLayers:(MaplyTileID)tileID
{
    NSMutableArray *layerData = [NSMutableArray array];
    
    for (NSString *layerName in _layerNames)
    {
        MaplyVectorObject *vecObj = [self readTile:tileID layer:layerName];
        if (vecObj)
            [layerData addObject:vecObj];
    }
    
    return layerData;
}

// Turn the layer data into visuals
- (void)processLayers:(MaplyTileID)tileID layerData:(NSArray *)layerData layer:(MaplyQuadPagingLayer *)layer
{
    VecsForStyles vecsForStyles;
    
    // Work through the layers we might expect to find
    // We'll collect the vectors for each of the styles we encounter
    for (MaplyVectorObject *vecObj in layerData)
    {
        if (vecObj)
        {
            NSArray *vecObjs = [vecObj splitVectors];
            for (MaplyVectorObject *thisVecObj in vecObjs)
            {
                NSDictionary *vecDict = thisVecObj.attributes;
                // Look through the styles
                int foundStyles = 0;
                for (unsigned int si=0;si<100;si++)
                {
                    NSString *styleName = [NSString stringWithFormat:@"style%d",si];
                    NSString *styleUUID = vecDict[styleName];
                    if (![styleUUID isKindOfClass:[NSString class]])
                        break;
                    
                    // Add the vector to the appropriate spot for the style
                    std::string uuid = [styleUUID cStringUsingEncoding:NSASCIIStringEncoding];
                    VecsForStyles::iterator it = vecsForStyles.find(uuid);
                    NSMutableArray *vecsForThisStyle = nil;
                    if (it == vecsForStyles.end())
                    {
                        vecsForThisStyle = [NSMutableArray array];
                        vecsForStyles[uuid] = vecsForThisStyle;
                    } else
                        vecsForThisStyle = it->second;
                    [vecsForThisStyle addObject:thisVecObj];
                    foundStyles++;
                }
                if (foundStyles == 0)
                    NSLog(@"Found data without style.");
            }
        }
    }
    
    // Work through the styles we found, adding everything together
    for (VecsForStyles::iterator it = vecsForStyles.begin();it != vecsForStyles.end(); ++it)
    {
        // Get the style object and then add the data
        MaplyVectorTileStyle *style = [self getStyle:it->first];
        NSArray *compObjs = [style buildObjects:it->second forTile:tileID viewC:layer.viewC];
        if (compObjs)
        {
            [layer addData:compObjs forTile:tileID style:(style.geomAdditive ? MaplyDataStyleAdd : MaplyDataStyleReplace)];
        }
    }
}

- (void)startFetchForTile:(MaplyTileID)tileID forLayer:(MaplyQuadPagingLayer *)layer
{
//    NSLog(@"Vector Fetching: %d: (%d, %d)",tileID.level,tileID.x,tileID.y);
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
       ^{
           // Need to do a network fetch
           if (tileURLs)
           {
               // See if it's in the cache first
               NSString *cacheFileName = nil;
               NSData *cacheData = nil;
               if (_cacheDir)
               {
                   cacheFileName = [NSString stringWithFormat:@"%@/tile%d_%d_%d",_cacheDir,tileID.level,tileID.x,tileID.y];
                   cacheData = [NSData dataWithContentsOfFile:cacheFileName];
               }
               
               if (cacheData)
               {
                   NSData *uncompressedData = [cacheData uncompressGZip];
                   MaplyVectorObject *vecObj = nil;
                   if ([uncompressedData length] > 0)
                       vecObj = [MaplyVectorObject VectorObjectFromVectorDBRaw:uncompressedData];
                   if (vecObj)
                       [self processLayers:tileID layerData:@[vecObj] layer:layer];
                   
//                   NSLog(@"Loaded tile: %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
                   [layer tileDidLoad:tileID];
               } else {
                   // Decide here which URL we'll use
                   NSString *tileURL = [tileURLs objectAtIndex:tileID.x%[tileURLs count]];
                   
                   // Use the JSON tile spec
    //               int y = ((int)(1<<tileID.level)-tileID.y)-1;
                   NSString *fullURLStr = [[[tileURL stringByReplacingOccurrencesOfString:@"{z}" withString:[@(tileID.level) stringValue]]
                                            stringByReplacingOccurrencesOfString:@"{x}" withString:[@(tileID.x) stringValue]]
                                           stringByReplacingOccurrencesOfString:@"{y}" withString:[@(tileID.y) stringValue]];
                   NSMutableURLRequest *urlReq = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:fullURLStr]];
                   urlReq.cachePolicy = NSURLRequestReloadIgnoringLocalCacheData;
                   // Note: Should set the timeout
                   
                   AFHTTPRequestOperation *op = [[AFHTTPRequestOperation alloc] initWithRequest:urlReq];
                   dispatch_queue_t runQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
                   op.completionQueue = runQueue;
                   [op setCompletionBlockWithSuccess:
                    ^(AFHTTPRequestOperation *operation, id responseObject)
                    {
                        NSData *tileData = responseObject;
                        if ([tileData isKindOfClass:[NSData class]])
                        {
                            // Uncompress the data
                            NSData *uncompressedData = [tileData uncompressGZip];
                            MaplyVectorObject *vecObj = nil;
                            if ([uncompressedData length] > 0)
                                vecObj = [MaplyVectorObject VectorObjectFromVectorDBRaw:uncompressedData];
                            if (vecObj)
                                [self processLayers:tileID layerData:@[vecObj] layer:layer];
                     
//                            NSLog(@"Loaded tile: %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
                            
                            // Save out to the cache
                            if (cacheFileName)
                                if (![tileData writeToFile:cacheFileName atomically:YES])
                                    NSLog(@"Failed to write tile: %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
                            
                            [layer tileDidLoad:tileID];
                        } else {
    //                        NSLog(@"Got unexpected data back from vector tile request for %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
                            [layer tileFailedToLoad:tileID];
                        }
                    }
                                             failure:
                    ^(AFHTTPRequestOperation *operation, NSError *error)
                    {
    //                    NSLog(@"Failed to fetch vector tile for %d: (%d,%d)\n%@",tileID.level,tileID.x,tileID.y,error);
                        // Note: We have to do this because we're missing tiles in the middle
                        [layer tileDidLoad:tileID];
    //                    [layer tileFailedToLoad:tileID];
                    }];
                   // Note: Should track this so we can cancel it
                   //       Go look at the remote image tile logic
                   [op start];
               }
           } else {
               // Fetch locally all at once.  Makes the logic simpler
               NSArray *layerData = [self fetchLayers:tileID];
               
               if ([layerData count] > 0)
                   [self processLayers:tileID layerData:layerData layer:layer];

//           NSLog(@"Loaded: %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
               [layer tileDidLoad:tileID];
           }
       });
}

@end