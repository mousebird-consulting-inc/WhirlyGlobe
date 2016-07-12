//
//  ViewController.m
//  HelloEarth
//
//  Created by Steve Gifford on 11/11/14.
//  Copyright (c) 2014 mousebird consulting. All rights reserved.
//

#import "ViewController.h"
#import "WhirlyGlobeComponent.h"

@interface ViewController ()

@end

@implementation ViewController
{
    MaplyBaseViewController *theViewC;
    WhirlyGlobeViewController *globeViewC;
    MaplyViewController *mapViewC;
}

// Set this to false for a map
const bool DoGlobe = true;

- (void)viewDidLoad {
    [super viewDidLoad];
    
    if (DoGlobe)
    {
        globeViewC = [[WhirlyGlobeViewController alloc] init];
        theViewC = globeViewC;
    } else {
        mapViewC = [[MaplyViewController alloc] init];
        theViewC = mapViewC;
    }

    // Create an empty globe or map and add it to the view
    [self.view addSubview:theViewC.view];
    theViewC.view.frame = self.view.bounds;
    [self addChildViewController:theViewC];
    
    // we want a black background for a globe, a white background for a map.
    theViewC.clearColor = (globeViewC != nil) ? [UIColor blackColor] : [UIColor whiteColor];
    
    // and thirty fps if we can get it ­ change this to 3 if you find your app is struggling
    theViewC.frameInterval = 2;
    
    // set up the data source
    MaplyMBTileSource *tileSource = [[MaplyMBTileSource alloc]
                                     initWithMBTiles:@"geography-class_medres"];
    
    // set up the layer
    MaplyQuadImageTilesLayer *layer =
    [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:tileSource.coordSys
                                               tileSource:tileSource];
    layer.handleEdges = (globeViewC != nil);
    layer.coverPoles = (globeViewC != nil);
    layer.requireElev = false;
    layer.waitLoad = false;
    layer.drawPriority = 0;
    layer.singleLevelLoading = false;
    [theViewC addLayer:layer];
    
    // start up over San Francisco
    if (globeViewC != nil)
    {
        globeViewC.height = 0.8;
        [globeViewC animateToPosition:MaplyCoordinateMakeWithDegrees(-122.4192,37.7793)
                                 time:1.0];
    } else {
        mapViewC.height = 1.0;
        [mapViewC animateToPosition:MaplyCoordinateMakeWithDegrees(-122.4192,37.7793)
                               time:1.0];
    }
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
