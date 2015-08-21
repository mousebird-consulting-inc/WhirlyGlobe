//
//  PagingTestDelegate.m
//  WhirlyGlobeComponentTester
//
//  Created by Steve Gifford on 8/17/15.
//  Copyright (c) 2015 mousebird consulting. All rights reserved.
//

#import "PagingTestDelegate.h"

@implementation PagingTestDelegate
{
    UIImage *image;
}

- (id)init
{
    self = [super init];
    image = [UIImage imageNamed:@"map_pin"];
    _coordSys = [[MaplySphericalMercator alloc] initWebStandard];
    
    return self;
}

- (int)minZoom
{
    return 0;
}

- (int)maxZoom
{
    return 22;
}

static int NumMarkers = 200;
static const float MaxDelay = 1.0;

- (void)startFetchForTile:(MaplyTileID)tileID forLayer:(MaplyQuadPagingLayer *)layer
{
    MaplyBoundingBox bbox;
    [layer geoBoundsforTile:tileID ll:&bbox.ll ur:&bbox.ur];
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                   ^{
                       // Random delay
                       usleep(drand48()* MaxDelay * 1e6);
                       
                       NSMutableArray *markers = [NSMutableArray array];
                       for (unsigned int ii=0;ii<NumMarkers;ii++)
                       {
                           MaplyScreenMarker *marker = [[MaplyScreenMarker alloc] init];
                           marker.layoutImportance = MAXFLOAT;
                           marker.image = image;
                           marker.size = CGSizeMake(32.0, 32.0);
                           marker.loc = MaplyCoordinateMake((bbox.ur.x-bbox.ll.x)*drand48()+bbox.ll.x, (bbox.ur.y-bbox.ll.y)*drand48()+bbox.ll.y);
                           [markers addObject:marker];
                       }

                       MaplyComponentObject *compObj = [layer.viewC addScreenMarkers:markers desc:@{kMaplyEnable: @(NO)} mode:MaplyThreadCurrent];
                       [layer addData:@[compObj] forTile:tileID];
                       [layer tileDidLoad:tileID];
                   });
}

@end
