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
#import <string>
#import <map>
#import <vector>

using namespace Maply;

typedef std::map<std::string,MaplyVectorTileStyle *> StyleMap;

@implementation MaplyVectorTiles
{
    // If we're reading from a database, these are set
    FMDatabase *db;
    FMDatabaseQueue *queue;
    bool compressed;
    std::vector<VectorAttribute> vecAttrs;
    
    // The style info
    StyleMap styleObjects;
}

// Parse a UIColor from hex values
+(UIColor *) ParseColor:(NSString *)colorStr
{
    int red = 255, green = 255, blue = 255;
    float alpha = 1.0;
    // parse the hex
    NSScanner *scanner = [NSScanner scannerWithString:colorStr];
    unsigned int colorVal;
    [scanner setScanLocation:1]; // bypass #
    [scanner scanHexInt:&colorVal];
    blue = colorVal & 0xFF;
    green = (colorVal >> 8) & 0xFF;
    red = (colorVal >> 16) & 0xFF;
    
    return [UIColor colorWithRed:red/255.0*alpha green:green/255.0*alpha blue:blue/255.0*alpha alpha:alpha];
}

- (id)initWithDirectory:(NSString *)tilesDir
{
    self = [super init];
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
    _minLevel = [paramDict[@"minLevel"] integerValue];
    _maxLevel = [paramDict[@"maxLevel"] integerValue];
    
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

- (id)initWithDatabase:(NSString *)name
{
    self = [super init];
    
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
    
    // Vector attribute names and types
    res = [db executeQuery:@"SELECT name,type FROM attributes"];
    while ([res next])
    {
        NSString *name = [res stringForColumn:@"name"];
        int type = [res intForColumn:@"type"];
        VectorAttribute attr;
        attr.name = name;
        attr.type = (VectorAttributeType)type;
        if (type >= VectorAttrMax)
            return nil;
        vecAttrs.push_back(attr);
    }
    
    _settings = [[MaplyVectorTileStyleSettings alloc] init];
    _settings.lineScale = [UIScreen mainScreen].scale;
    _settings.textScale = [UIScreen mainScreen].scale;
    _settings.markerScale = [UIScreen mainScreen].scale;    
    
    return self;
}

- (int)minZoom
{
    return _minLevel;
}

- (int)maxZoom
{
    return _maxLevel;
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
            MaplyVectorTileStyle *styleObj = [MaplyVectorTileStyle styleFromStyleEntry:theStyle settings:_settings];
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
            vecObj = [MaplyVectorObject VectorObjectFromVectorDBRaw:uncompressedData attrs:&vecAttrs];
        }
    } else {
        NSString *fileName = [NSString stringWithFormat:@"%@/%d/%d/%d%@",_tilesDir,tileID.level,tileID.y,tileID.x,layerName];
        vecObj = [MaplyVectorObject VectorObjectFromShapeFile:fileName];
    }

    // Note: Debugging
//    if (vecObj)
//    {
//        int numPoints=0, numLinears=0, numAreals=0;
//        [self countVecs:vecObj points:&numPoints linears:&numLinears areals:&numAreals];
//        NSLog(@"Loading tile: %@ %d: (%d,%d)\n\tpoints = %d, linears = %d, areals = %d",layerName,tileID.level,tileID.x,tileID.y,numPoints,numLinears,numAreals);
//    }

    return vecObj;
}

typedef std::map<std::string,NSMutableArray *> VecsForStyles;

- (void)startFetchForTile:(MaplyTileID)tileID forLayer:(MaplyQuadPagingLayer *)layer
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
       ^{
           VecsForStyles vecsForStyles;
           
           // Work through the layers we might expect to find
           // We'll collect the vectors for each of the styles we encounter
           for (NSString *layerName in _layerNames)
           {
               MaplyVectorObject *vecObj = [self readTile:tileID layer:layerName];
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
               NSArray *compObjs = [style buildObjects:it->second viewC:layer.viewC];
               if (compObjs)
                   [layer addData:compObjs forTile:tileID style:(style.geomAdditive ? MaplyDataStyleAdd : MaplyDataStyleReplace)];
           }

           
           [layer tileDidLoad:tileID];
       });
}

@end