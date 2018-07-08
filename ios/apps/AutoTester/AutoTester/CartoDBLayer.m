//
//  CartoDBLayer.m
//  HelloEarth
//
//  Created by Steve Gifford on 11/18/14.
//  Copyright © 2014-2017 mousebird consulting. All rights reserved.
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

- (NSURLRequest *)constructRequest:(MaplyBoundingBox)bbox
{
	// construct a query string
	double toDeg = 180/M_PI;
	NSString *query = [NSString stringWithFormat:search,bbox.ll.x*toDeg,bbox.ll.y*toDeg,bbox.ur.x*toDeg,bbox.ur.y*toDeg];
    NSString *encodeQuery = [query stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet URLHostAllowedCharacterSet]];
	encodeQuery = [encodeQuery stringByReplacingOccurrencesOfString:@"&" withString:@"%26"];
	NSString *fullUrl = [NSString stringWithFormat:@"https://pluto.cartodb.com/api/v2/sql?format=GeoJSON&q=%@",encodeQuery];
	NSURLRequest *urlReq = [NSURLRequest requestWithURL:[NSURL URLWithString:fullUrl]];
	
	return urlReq;
}

- (void)startFetchForTile:(MaplyTileID)tileID forLayer:(MaplyQuadPagingLayer *)layer
{
	// bounding box for tile
	MaplyBoundingBox bbox;
	[layer geoBoundsforTile:tileID ll:&bbox.ll ur:&bbox.ur];
	NSURLRequest *urlReq = [self constructRequest:bbox];
	
    NSURLSession *session = [NSURLSession sharedSession];
    NSURLSessionDataTask *task = [session dataTaskWithRequest:urlReq completionHandler:
                                  ^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {
                                      dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
                                          if (error) {
                                              [layer tileFailedToLoad:tileID];
                                              return;
                                          }

                                          // parse the resulting GeoJSON
                                          MaplyVectorObject *vecObj = [MaplyVectorObject VectorObjectFromGeoJSON:data];
                                          if (vecObj)
                                          {
                                              // display it on the map
                                              MaplyComponentObject *filledObj =
                                              [layer.viewC addVectors:@[vecObj]
                                                                 desc:@{kMaplyColor: [UIColor colorWithRed:0.25 green:0.0 blue:0.0 alpha:0.25],
                                                                        kMaplyFilled: @(YES),
                                                                        kMaplyEnable: @(NO)
                                                                        }
                                                                 mode:MaplyThreadCurrent];
                                              MaplyComponentObject *outlineObj =
                                              [layer.viewC addVectors:@[vecObj]
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
                                      });
                            }];
    [task resume];
}

@end
