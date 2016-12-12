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
    MaplyRemoteTileSource *tileSource = [[MaplyRemoteTileSource alloc] initWithBaseURL:@"http://tile.stamen.com/terrain/" ext:@"png" minZoom:0 maxZoom:18];
    tileSource.cacheDir = [NSString stringWithFormat:@"%@/tiles/",cacheDir];
    MaplyQuadImageTilesLayer *layer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:tileSource.coordSys tileSource:tileSource];
    layer.drawPriority = 100;
    layer.handleEdges = true;
    [globeVC addLayer:layer];

    // Add Micello copyright notice
    UILabel *copyrightLabel = [[UILabel alloc] init];
    [copyrightLabel setTranslatesAutoresizingMaskIntoConstraints:NO];
    copyrightLabel.text = @"Westfield Valley Fair Map Data © Micello";
    copyrightLabel.backgroundColor = [UIColor colorWithWhite:1.0 alpha:0.5];
    copyrightLabel.textAlignment = NSTextAlignmentRight;
    [globeVC.view addSubview:copyrightLabel];
    [globeVC.view addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"[copyrightLabel(310)]-20-|" options:0 metrics:nil views:NSDictionaryOfVariableBindings(copyrightLabel)]];
    [globeVC.view addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:[copyrightLabel(25)]-20-|" options:0 metrics:nil views:NSDictionaryOfVariableBindings(copyrightLabel)]];
}

@end
