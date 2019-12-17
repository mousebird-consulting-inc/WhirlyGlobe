//
//  CartoDBLayer.m
//  HelloEarth
//
//  Created by Steve Gifford on 11/18/14.
//  Copyright © 2014-2017 mousebird consulting.
//

#import "CartoDBInterpreter.h"

@implementation CartoDBInterpreter
{
    NSString *search;
}

- (id)initWithSearch:(NSString *)inSearch
{
	self = [super init];
	search = inSearch;
	
	return self;
}

- (void)setLoader:(MaplyQuadLoaderBase * _Nonnull)loader {
    _loader = (MaplyQuadPagingLoader *)loader;
}

// Generate a URL for a given tile
- (MaplyRemoteTileFetchInfo *)fetchInfoForTile:(MaplyTileID)tileID flipY:(bool)flipY
{
    // bounding box for tile
    MaplyBoundingBox bbox = [_loader geoBoundsForTile:tileID];

    MaplyRemoteTileFetchInfo *fetchInfo = [[MaplyRemoteTileFetchInfo alloc] init];

    // construct a query string
    double toDeg = 180/M_PI;
    NSString *query = [NSString stringWithFormat:search,bbox.ll.x*toDeg,bbox.ll.y*toDeg,bbox.ur.x*toDeg,bbox.ur.y*toDeg];
    NSString *encodeQuery = [query stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet URLHostAllowedCharacterSet]];
    encodeQuery = [encodeQuery stringByReplacingOccurrencesOfString:@"&" withString:@"%26"];
    NSString *fullUrl = [NSString stringWithFormat:@"https://pluto.cartodb.com/api/v2/sql?format=GeoJSON&q=%@",encodeQuery];
    NSURLRequest *urlReq = [NSURLRequest requestWithURL:[NSURL URLWithString:fullUrl]];

    fetchInfo.urlReq = urlReq;
    return fetchInfo;
}

- (void)dataForTile:(MaplyObjectLoaderReturn *)loadReturn loader:(MaplyQuadLoaderBase *)loader
{
    NSData *data = loadReturn.getFirstData;
    if (!data)
        return;
    
    // parse the resulting GeoJSON
    MaplyVectorObject *vecObj = [MaplyVectorObject VectorObjectFromGeoJSON:data];
    if (vecObj)
    {
        // display it on the map
        MaplyComponentObject *filledObj =
        [_loader.viewC addVectors:@[vecObj]
                             desc:@{kMaplyColor: [UIColor colorWithRed:0.25 green:0.0 blue:0.0 alpha:0.25],
                                    kMaplyFilled: @(YES),
                                    kMaplyEnable: @(NO)
                             }
                             mode:MaplyThreadCurrent];
        MaplyComponentObject *outlineObj =
        [_loader.viewC addVectors:@[vecObj]
                             desc:@{kMaplyColor: [UIColor redColor],
                                    kMaplyFilled: @(NO),
                                    kMaplyEnable: @(NO)
                             }
                             mode:MaplyThreadCurrent];
        
        [loadReturn addCompObj:filledObj];
        [loadReturn addCompObj:outlineObj];
    }
}

- (void)tileUnloaded:(MaplyTileID)tileID { 
}


@end
