//
//  MapzenSource.m
//  WhirlyGlobeComponentTester
//
//  Created by Steve Gifford on 11/20/14.
//  Copyright (c) 2014 mousebird consulting. All rights reserved.
//

#import "MapzenSource.h"
#import "MaplyMapnikVectorTiles.h"

@implementation MapzenSource
{
    NSString *baseURL;
    NSArray *layers;
    NSOperationQueue *opQueue;
    NSString *ext;
    MaplyMapnikVectorTileParser *tileParser;
    NSObject<MaplyVectorStyleDelegate> *styleSet;
}

- (id)initWithBase:(NSString *)inBaseURL layers:(NSArray *)inLayers styleData:(NSData *)styleData type:(MapzenSourceType)inType viewC:(MaplyBaseViewController *)viewC
{
    self = [super init];
    if (!self)
        return nil;
    baseURL = inBaseURL;
    layers = inLayers;
    opQueue = [[NSOperationQueue alloc] init];
    
    switch (inType)
    {
        case MapzenSourceGeoJSON:
            ext = @"json";
            break;
        case MapzenSourcePBF:
        {
            ext = @"mapbox";
            // Parse the style sheet
            MapnikStyleSet *mapnikStyleSet = [[MapnikStyleSet alloc] initForViewC:viewC];
            [mapnikStyleSet loadXmlData:styleData];
            [mapnikStyleSet generateStyles];
            styleSet = mapnikStyleSet;
            
            // Create a tile parser for later
            tileParser = [[MaplyMapnikVectorTileParser alloc] initWithStyle:styleSet viewC:viewC];
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
                     if (tileParser)
                     {
                         MaplyBoundingBox bbox;
                         // The tile parser wants bounds in meters(ish)
                         [layer boundsforTile:tileID ll:&bbox.ll ur:&bbox.ur];
                         bbox.ll.x *= 20037508.342789244/M_PI;
                         bbox.ll.y *= 20037508.342789244/(M_PI);
                         bbox.ur.x *= 20037508.342789244/M_PI;
                         bbox.ur.y *= 20037508.342789244/(M_PI);

                         // Expecting vector.pbf
                         [tileParser buildObjects:data tile:tileID bounds:bbox];
                     } else {
                         // Expecting GeoJSON
                         MaplyVectorObject *vecObj = [MaplyVectorObject VectorObjectFromGeoJSON:data];

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
                 }
                 
                 [layer tileDidLoad:tileID part:partID];
             }];
            
            partID++;
        }
    });
}

@end
