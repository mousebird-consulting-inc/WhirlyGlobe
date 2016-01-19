//
//  MapBoxVectorTestCase.m
//  AutoTester
//
//  Created by jmnavarro on 26/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

#import "MapBoxVectorTestCase.h"
#import "MaplyMapnikVectorTiles.h"
#import "MaplyViewController.h"
#import "AutoTester-Swift.h"
#import "MapboxVectorStyleSet.h"

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
	MapBoxSatelliteTestCase *gctc = [[MapBoxSatelliteTestCase alloc]init];
	[gctc setUpWithMap:mapVC];

	// For network paging layers, where we'll store temp files
	NSString *cacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)  objectAtIndex:0];
	NSString *thisCacheDir = [NSString stringWithFormat:@"%@/mapbox-streets-vectiles",cacheDir];

	NSString *token = @"sk.eyJ1IjoiZG1hcnRpbnciLCJhIjoiY2lnYmViYmhiMDZmbWFha25kbHB3MWlkNyJ9.5VsRqKZvrTQ9ygnyI7fLoA";

	[MaplyMapnikVectorTiles StartRemoteVectorTilesWithTileSpec:@"https://a.tiles.mapbox.com/v4/mapbox.mapbox-streets-v6.json"
		accessToken:token
		style:@"https://raw.githubusercontent.com/mapbox/mapbox-gl-styles/master/styles/emerald-v8.json"
		styleType:MapnikMapboxGLStyle
		cacheDir:thisCacheDir
		viewC:(MaplyBaseViewController*)mapVC
		success:^(MaplyMapnikVectorTiles * _Nonnull vecTiles) {
			MaplyMapboxVectorStyleSet *styleSet = (MaplyMapboxVectorStyleSet *) vecTiles.tileParser.styleDelegate;
			styleSet.tileStyleSettings.markerImportance = 10.0;
			styleSet.tileStyleSettings.fontName = @"Gill Sans";
			UIColor *backColor = [styleSet backgroundColor];
			if (backColor) {
				[mapVC setClearColor:backColor];
			}

			// Now for the paging layer itself
			MaplyQuadPagingLayer *pageLayer = [[MaplyQuadPagingLayer alloc] initWithCoordSystem:[[MaplySphericalMercator alloc] initWebStandard] delegate:vecTiles];
			pageLayer.numSimultaneousFetches = 6;
			pageLayer.flipY = false;
			pageLayer.importance = 1024*1024*2;
			pageLayer.useTargetZoomLevel = true;
			pageLayer.singleLevelLoading = true;
			[mapVC addLayer:pageLayer];

			[mapVC animateToPosition:MaplyCoordinateMakeWithDegrees(-122.290,37.7793) height:0.0005 time:0.1];
		} failure:^(NSError * _Nonnull error) {
			NSLog(@"Failed to load Mapnik vector tiles because: %@",error);
		}];

	return true;
}

@end
