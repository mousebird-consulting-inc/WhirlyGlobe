//
//  CartoDBLayer.m
//  HelloEarth
//
//  Created by Chris Lamb on 6/28/15.
//  Copyright (c) 2015 com.SantaCruzNewspaperTaxi. All rights reserved.
//

#import "CartoDBLayer.h"

@implementation CartoDBLayer

- (id)initWithSearch:(NSString *)inSearch
{
    self = [super init];
    search = inSearch;
    opQueue = [[NSOperationQueue alloc] init];
    
    return self;
}

- (void)startFetchForTile:(MaplyTileID)tileID forLayer:(MaplyQuadPagingLayer *)layer
{
    // bounding box for tile
    MaplyBoundingBox bbox;
    [layer geoBoundsforTile:tileID ll:&bbox.ll ur:&bbox.ur];
    NSURLRequest *urlReq = [self constructRequest:bbox];
    
    // kick off the query asychronously
    [NSURLConnection
     sendAsynchronousRequest:urlReq
     queue:opQueue
     completionHandler:
     ^(NSURLResponse *response, NSData *data, NSError *connectionError)
     {
         // parse the resulting GeoJSON
         MaplyVectorObject *vecObj = [MaplyVectorObject VectorObjectFromGeoJSON:data];
         if (vecObj)
         {
             // display a transparent filled polygon
             MaplyComponentObject *filledObj =
             [layer.viewC
              addVectors:@[vecObj]
              desc:@{kMaplyColor: [UIColor colorWithRed:0.25
                                                  green:0.0 blue:0.0 alpha:0.25],
                     kMaplyFilled: @(YES),
                     kMaplyEnable: @(NO)
                     }
              mode:MaplyThreadCurrent];
             
             // display a line around the lot
             MaplyComponentObject *outlineObj =
             [layer.viewC
              addVectors:@[vecObj]
              desc:@{kMaplyColor: [UIColor redColor],
                     kMaplyFilled: @(NO),
                     kMaplyEnable: @(NO)
                     }
              mode:MaplyThreadCurrent];
             
             // keep track of it in the layer
             [layer addData:@[filledObj,outlineObj] forTile:tileID];
         }
         
         // let the layer know the tile is done
         [layer tileDidLoad:tileID];
     }];
}

- (NSURLRequest *)constructRequest:(MaplyBoundingBox)bbox
{
    double toDeg = 180/M_PI;
    NSString *query = [NSString stringWithFormat:search,bbox.ll.x*toDeg,bbox.ll.y*toDeg,bbox.ur.x*toDeg,bbox.ur.y*toDeg];
    NSString *encodeQuery = [query stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
    encodeQuery = [encodeQuery stringByReplacingOccurrencesOfString:@"&" withString:@"%26"];
    NSString *fullUrl = [NSString stringWithFormat:@"https://pluto.cartodb.com/api/v2/sql?format=GeoJSON&q=%@",encodeQuery];
    NSURLRequest *urlReq = [NSURLRequest requestWithURL:[NSURL URLWithString:fullUrl]];
    
    return urlReq;
}

@end
