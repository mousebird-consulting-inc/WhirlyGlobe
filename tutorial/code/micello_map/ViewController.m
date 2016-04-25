//
//  ViewController.m
//  WhirlyGlobeMicelloDemo
//
//  Created by Ranen Ghosh on 2016-04-14.
//  Copyright 2011-2016 mousebird consulting
//

#import "ViewController.h"
#import "MaplyMicelloMap.h"
#import "SimpleAnnotationViewController.h"

@implementation ViewController
{
    WhirlyGlobeViewController *globeVC;
}
- (void)viewDidLoad {
    [super viewDidLoad];
    
    // Create an empty globe and add it to the view
    globeVC = [[WhirlyGlobeViewController alloc] init];
    globeVC.delegate = self;
    [self.view addSubview:globeVC.view];
    globeVC.view.frame = self.view.bounds;
    [self addChildViewController:globeVC];
    
    // Add OpenStreetMap basemap
    NSString *cacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)  objectAtIndex:0];
    MaplyRemoteTileSource *tileSource = [[MaplyRemoteTileSource alloc] initWithBaseURL:@"http://otile1.mqcdn.com/tiles/1.0.0/osm/" ext:@"png" minZoom:0 maxZoom:18];
    tileSource.cacheDir = [NSString stringWithFormat:@"%@/osmtiles/",cacheDir];
    MaplyQuadImageTilesLayer *layer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:tileSource.coordSys tileSource:tileSource];
    layer.drawPriority = 100;
    layer.handleEdges = true;
    [globeVC addLayer:layer];
    
}

@end
