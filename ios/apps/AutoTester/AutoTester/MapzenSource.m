//
//  MapzenSource.m
//  WhirlyGlobeComponentTester
//
//  Created by Steve Gifford on 11/20/14.
//  Copyright © 2014-2017 mousebird consulting. All rights reserved.
//

#import "MapzenSource.h"
#import "MapboxVectorTiles.h"
#import "MapnikStyleSet.h"
#import "SLDStyleSet.h"

@implementation MapzenSource
{
    NSString *baseURL;
    NSArray *layers;
    NSString *apiKey;
    NSOperationQueue *opQueue;
    NSString *ext;
    MapboxVectorTileParser *tileParser;
    NSObject<MaplyVectorStyleDelegate> *styleSet;
}
- (id)initWithBase:(NSString *)inBaseURL layers:(NSArray *)inLayers apiKey:(NSString *)inApiKey sourceType:(MapzenSourceType)inType styleData:(NSData *)styleData styleType:(MapnikStyleType)styleType viewC:(MaplyBaseViewController *)viewC
{
    self = [super init];
    if (!self)
        return nil;
    baseURL = inBaseURL;
    layers = inLayers;
    apiKey = inApiKey;
    opQueue = [[NSOperationQueue alloc] init];
    
    switch (inType)
    {
        case MapzenSourceGeoJSON:
            ext = @"json";
            break;
        case MapzenSourcePBF:
        {
            ext = @"mvt";

            switch (styleType)
            {
                case MapnikXMLStyle:
                {
                    MapnikStyleSet *mapnikStyleSet = [[MapnikStyleSet alloc] initForViewC:viewC];
                    [mapnikStyleSet loadXmlData:styleData];
                    [mapnikStyleSet generateStyles];
                    styleSet = mapnikStyleSet;
                }
                    break;
                case MapnikSLDStyle:
                {
                    // The simple version will display everything
//                    MaplyVectorStyleSimpleGenerator *simpleSet = [[MaplyVectorStyleSimpleGenerator alloc] initWithViewC:viewC];
//                    styleSet = simpleSet;
                    // This version uses an SLD
                    SLDStyleSet *sldStyleSet = [[SLDStyleSet alloc] initWithViewC:viewC useLayerNames:NO relativeDrawPriority:0];
                    [sldStyleSet loadSldData:styleData baseURL:[NSURL URLWithString:baseURL]];
                    styleSet = sldStyleSet;
                }
                    break;
            }
                        
            // Create a tile parser for later
            tileParser = [[MapboxVectorTileParser alloc] initWithStyle:styleSet viewC:viewC];
        }
            break;
    }
    
    return self;
}

- (void)startFetchForTile:(MaplyTileID)tileID forLayer:(MaplyQuadPagingLayer *)layer
{
    int y = (1<<tileID.level)-tileID.y-1;
    NSString *cacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)  objectAtIndex:0];

    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
    ^{
        if (tileParser)
        {
            // We can fetch one vector.pbf
            NSMutableString *allLayers = [NSMutableString string];
            for (NSString *layerName in layers)
            {
                if ([allLayers length] > 0)
                    [allLayers appendString:@","];
                [allLayers appendString:layerName];
            }
            
            NSString *fullUrl = [NSString stringWithFormat:@"%@/%@/%d/%d/%d.%@",baseURL,allLayers,tileID.level,tileID.x,y,ext];
            if (apiKey)
                fullUrl = [NSString stringWithFormat:@"%@?api_key=%@",fullUrl,apiKey];
            NSString *fileName = [NSString stringWithFormat:@"%@_level%d_%d_%d.%@",allLayers,tileID.level,tileID.x,y,ext];
            NSString *fullPath = [NSString stringWithFormat:@"%@/%@",cacheDir,fileName];
            
            MaplyBoundingBox bbox;
            // The tile parser wants bounds in meters(ish)
            [layer boundsforTile:tileID ll:&bbox.ll ur:&bbox.ur];
            bbox.ll.x *= 20037508.342789244/M_PI;
            bbox.ll.y *= 20037508.342789244/(M_PI);
            bbox.ur.x *= 20037508.342789244/M_PI;
            bbox.ur.y *= 20037508.342789244/(M_PI);

            if ([[NSFileManager defaultManager] fileExistsAtPath:fullPath])
            {
                NSData *data = [NSData dataWithContentsOfFile:fullPath];
                MaplyVectorTileData *tileData = [tileParser buildObjects:data tile:tileID bounds:bbox];
                if (tileData.compObjs)
                    [layer addData:tileData.compObjs forTile:tileID];
                
                [layer tileDidLoad:tileID];
            } else {
                NSURL *url = [NSURL URLWithString:fullUrl];
                NSURLRequest *urlRequest = [NSURLRequest requestWithURL:url];
                [NSURLConnection sendAsynchronousRequest:urlRequest queue:opQueue completionHandler:
                 ^(NSURLResponse *response, NSData *data, NSError *connectionError)
                 {
                     if (!connectionError)
                     {
                         // Cache the file
                         [data writeToFile:fullPath atomically:NO];
                         
                         MaplyVectorTileData *tileData = [tileParser buildObjects:data tile:tileID bounds:bbox];
                         if (tileData.compObjs)
                             [layer addData:tileData.compObjs forTile:tileID];
                     }
                     
                     [layer tileDidLoad:tileID];
                 }];
            }
        } else {
            // Fetch GeoJSON individually
            [layer tile:tileID hasNumParts:(int)[layers count]];
            
            // Work through the layers
            int partID = 0;
            for (NSString *layerName in layers)
            {
                NSString *fullUrl = [NSString stringWithFormat:@"%@/%@/%d/%d/%d.%@",baseURL,layerName,tileID.level,tileID.x,y,ext];
                NSString *fileName = [NSString stringWithFormat:@"%@_level%d_%d_%d.%@",layerName,tileID.level,tileID.x,y,ext];
                NSString *fullPath = [NSString stringWithFormat:@"%@/%@",cacheDir,fileName];
                NSURL *url = [NSURL URLWithString:fullUrl];
                NSURLRequest *urlRequest = [NSURLRequest requestWithURL:url];
                [NSURLConnection sendAsynchronousRequest:urlRequest queue:opQueue completionHandler:
                 ^(NSURLResponse *response, NSData *data, NSError *connectionError)
                 {
                     if (!connectionError)
                     {
                         // Expecting GeoJSON
                         MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] initWithGeoJSON:data];
                         
                         if (vecObj)
                         {
                             // Save it to storage
                             [data writeToFile:fullPath atomically:NO];
                             
                             // Display it
                             MaplyComponentObject *compObj = [layer.viewC addVectors:@[vecObj]
                                                                                desc:@{kMaplyEnable: @(NO)}
                                                                                mode:MaplyThreadCurrent];
                             if (compObj)
                                 [layer addData:@[compObj] forTile:tileID];
                         }
                     }
                     
                     [layer tileDidLoad:tileID part:partID];
                 }];
                
                partID++;
        }
        
        }
    });
}

@end
