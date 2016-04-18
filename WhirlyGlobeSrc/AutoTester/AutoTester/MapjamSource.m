//
//  MapjamSource.m
//  AutoTester
//
//  Created by Steve Gifford on 4/18/16.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

#import "MapjamSource.h"

@implementation MapjamSource
{
    NSString *baseURL;
    NSString *apiKey;
    MapboxVectorTileParser *tileParser;
    NSObject<MaplyVectorStyleDelegate> *styleSet;
    NSOperationQueue *opQueue;
    NSString *ext;
}

- (id)initWithBase:(NSString *)inBaseURL apiKey:(NSString *)inApiKey styleJSON:(NSData *)styleJSON viewC:(MaplyBaseViewController *)viewC
{
    self = [super init];
    if (!self)
        return nil;
    
    baseURL = inBaseURL;
    apiKey = inApiKey;
    opQueue = [[NSOperationQueue alloc] init];
    ext = @"mvt";
    
    // Set up the style based on the JSON style sheet
    MaplyMapboxVectorStyleSet *mapboxStyleSet = [[MaplyMapboxVectorStyleSet alloc] initWithJSON:styleJSON viewC:viewC];
    styleSet = mapboxStyleSet;

    // Find the background color
    for (MapboxVectorLayerBackground *backLayer in mapboxStyleSet.layers)
        if ([backLayer isKindOfClass:[MapboxVectorLayerBackground class]])
            _backgroundColor = backLayer.paint.color;
    
    // Create a tile parser for later
    tileParser = [[MapboxVectorTileParser alloc] initWithStyle:styleSet viewC:viewC];
    
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
                           NSString *fullUrl = [NSString stringWithFormat:@"%@/%@/%d/%d/%d.%@",baseURL,@"all",tileID.level,tileID.x,y,ext];
                           if (apiKey)
                               fullUrl = [NSString stringWithFormat:@"%@?access_token=%@",fullUrl,apiKey];
                           NSString *fileName = [NSString stringWithFormat:@"%@_level%d_%d_%d.%@",@"all",tileID.level,tileID.x,y,ext];
                           NSString *fullPath = [NSString stringWithFormat:@"%@/%@",cacheDir,fileName];
                           NSURL *url = [NSURL URLWithString:fullUrl];
                           NSURLRequest *urlRequest = [NSURLRequest requestWithURL:url];
                           [NSURLConnection sendAsynchronousRequest:urlRequest queue:opQueue completionHandler:
                            ^(NSURLResponse *response, NSData *data, NSError *connectionError)
                            {
                                if (!connectionError)
                                {
                                    MaplyBoundingBox bbox;
                                    // The tile parser wants bounds in meters(ish)
                                    [layer boundsforTile:tileID ll:&bbox.ll ur:&bbox.ur];
                                    bbox.ll.x *= 20037508.342789244/M_PI;
                                    bbox.ll.y *= 20037508.342789244/(M_PI);
                                    bbox.ur.x *= 20037508.342789244/M_PI;
                                    bbox.ur.y *= 20037508.342789244/(M_PI);
                                    
                                    // Cache the file
                                    [data writeToFile:fullPath atomically:NO];
                                    
                                    MaplyVectorTileData *tileData = [tileParser buildObjects:data tile:tileID bounds:bbox];
                                    if (tileData.compObjs)
                                        [layer addData:tileData.compObjs forTile:tileID];
                                }
                                
                                [layer tileDidLoad:tileID];
                            }];
                       }
                   });
}

@end
