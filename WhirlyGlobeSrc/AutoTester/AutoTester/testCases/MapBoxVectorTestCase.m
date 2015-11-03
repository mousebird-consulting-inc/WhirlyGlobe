//
//  MapBoxVectorTestCase.m
//  AutoTester
//
//  Created by jmnavarro on 26/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

#import "MapBoxVectorTestCase.h"
#import "MaplyMapnikVectorTiles.h"

@implementation MapBoxVectorTestCase


- (instancetype)init
{
	if (self = [super init]) {
		self.name = @"MapBox Vector";
		self.captureDelay = 5;
	}

	return self;
}

- (BOOL)setUpWithMap:(MaplyViewController *)mapVC
{
	// For network paging layers, where we'll store temp files
	NSString *cacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)  objectAtIndex:0];
	NSString *thisCacheDir = nil;
	thisCacheDir = [NSString stringWithFormat:@"%@/mapbox-streets-vectiles",cacheDir];
	[MaplyMapnikVectorTiles StartRemoteVectorTilesWithTileSpec:@"http://a.tiles.mapbox.com/v3/mapbox.mapbox-streets-v4.json"
			   style:[[NSBundle mainBundle] pathForResource:@"osm-bright" ofType:@"xml"]
			cacheDir:thisCacheDir
			   viewC:baseViewC
			 success: ^(MaplyMapnikVectorTiles *vecTiles) {
				// Don't load the lowest levels for the globe
				if (globeViewC)
				 vecTiles.minZoom = 5;

				// Note: These are set after the MapnikStyleSet has already been initialized
				MapnikStyleSet *styleSet = (MapnikStyleSet *)vecTiles.styleDelegate;
				styleSet.tileStyleSettings.markerImportance = 10.0;
				styleSet.tileStyleSettings.fontName = @"Gill Sans";

				// Now for the paging layer itself
				MaplyQuadPagingLayer *pageLayer = [[MaplyQuadPagingLayer alloc] initWithCoordSystem:[[MaplySphericalMercator alloc] initWebStandard] delegate:vecTiles];
				pageLayer.numSimultaneousFetches = 6;
				pageLayer.flipY = false;
				pageLayer.importance = 1024*1024*2;
				pageLayer.useTargetZoomLevel = true;
				pageLayer.singleLevelLoading = true;
				[baseViewC addLayer:pageLayer];
				ovlLayers[layerName] = pageLayer;
			}
			failure: ^(NSError *error) {
				NSLog(@"Failed to load Mapnik vector tiles because: %@",error);
			}
	];

	return true;
}

@end
