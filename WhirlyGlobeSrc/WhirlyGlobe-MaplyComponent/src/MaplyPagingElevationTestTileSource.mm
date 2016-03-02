/*
 *  MaplyPagingVectorTestTileSource.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 6/19/14.
 *  Copyright 2011-2015 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#import "MaplyScreenLabel.h"
#import "MaplyPagingElevationTestTileSource.h"
#import "ElevationCesiumChunk.h"
#import "MaplyElevationSource_private.h"

static const int MaxDebugColors = 10;
static const int debugColors[MaxDebugColors] = {0x86812D, 0x5EB9C9, 0x2A7E3E, 0x4F256F, 0xD89CDE, 0x773B28, 0x333D99, 0x862D52, 0xC2C653, 0xB8583D};

static UIColor* colorForElevatedSide(int zoomLevel, float darkFactor)
{
	int hexColor = debugColors[zoomLevel % MaxDebugColors];
	float red   = ((((hexColor) >> 16) & 0xFF)/255.0) * darkFactor;
	float green = ((((hexColor) >>  8) & 0xFF)/255.0) * darkFactor;
	float blue  = ((((hexColor) >>  0) & 0xFF)/255.0) * darkFactor;
	return [UIColor colorWithRed:red green:green blue:blue alpha:0.75];
}


@implementation MaplyPagingElevationTestTileSource

- (id)initWithCoordSys:(MaplyCoordinateSystem *)coordSys minZoom:(int)minZoom maxZoom:(int)maxZoom elevSource:(id<MaplyElevationSourceDelegate>) elevSource
{
    self = [super init];
    if (!self)
        return nil;
    
    _coordSys = coordSys;
    _minZoom = minZoom;
    _maxZoom = maxZoom;
	_elevSource = elevSource;
    
    return self;
}


- (void)startFetchForTile:(MaplyTileID)tileID forLayer:(MaplyQuadPagingLayer *)layer
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                   ^{
                       // Add in a little delay
                       usleep(0.215 * 1e6);
                       
                       if (tileID.level > _maxZoom)
                       {
                           [layer tileFailedToLoad:tileID];
                       } else {
                           MaplyCoordinate ll,ur;
                           [layer geoBoundsforTile:tileID ll:&ll ur:&ur];
                           MaplyCoordinate center;
                           center.x = (ll.x+ur.x)/2.0;  center.y = (ll.y+ur.y)/2.0;

                           // Color rectangle with outline
						   UIColor *color = colorForElevatedSide(tileID.level, 1.0);

                           MaplyCoordinate coords[3];
                           coords[0] = ll;
                           coords[1] = center;
                           coords[2].x = ll.x;
						   coords[2].y = ur.y;

                           MaplyVectorObject *rect1 = [[MaplyVectorObject alloc] initWithLineString:coords numCoords:3 attributes:nil];

                           MaplyComponentObject *compObj0 = [layer.viewC addVectors:@[rect1] desc:
                                                            @{kMaplyFilled: @(true),
                                                              kMaplyColor: color,
                                                              kMaplyDrawPriority: @(kMaplyVectorDrawPriorityDefault+100+tileID.level)
                                                              }
                                                            ];

						   color = colorForElevatedSide(tileID.level, 0.9);

                           coords[0] = ll;
                           coords[1].x = ur.x;
						   coords[1].y = ll.y;
                           coords[2] = center;
                           MaplyVectorObject *rect2 = [[MaplyVectorObject alloc] initWithLineString:coords numCoords:3 attributes:nil];

                           MaplyComponentObject *compObj1 = [layer.viewC addVectors:@[rect2] desc:
                                                            @{kMaplyFilled: @(true),
                                                              kMaplyColor: color,
                                                              kMaplyDrawPriority: @(kMaplyVectorDrawPriorityDefault+100+tileID.level)
                                                              }
                                                            ];

						   color = colorForElevatedSide(tileID.level, 0.8);

                           coords[0].x = ur.x;
						   coords[0].y = ll.y;
                           coords[1] = ur;
                           coords[2] = center;

                           rect1 = [[MaplyVectorObject alloc] initWithLineString:coords numCoords:3 attributes:nil];

                           MaplyComponentObject *compObj2 = [layer.viewC addVectors:@[rect1] desc:
                                                            @{kMaplyFilled: @(true),
                                                              kMaplyColor: color,
                                                              kMaplyDrawPriority: @(kMaplyVectorDrawPriorityDefault+100+tileID.level)
                                                              }
                                                            ];

						   color = colorForElevatedSide(tileID.level, 0.7);

                           coords[0] = ur;
                           coords[1].x = ll.x;
						   coords[1].y = ur.y;
                           coords[2] = center;
                           rect2 = [[MaplyVectorObject alloc] initWithLineString:coords numCoords:3 attributes:nil];
                           MaplyComponentObject *compObj3 = [layer.viewC addVectors:@[rect2] desc:
                                                            @{kMaplyFilled: @(true),
                                                              kMaplyColor: color,
                                                              kMaplyDrawPriority: @(kMaplyVectorDrawPriorityDefault+100+tileID.level)
                                                              }
                                                            ];

                           // Label

						   MaplyElevationChunk *elevChunk = [_elevSource elevForTile:tileID];
						   WhirlyKitElevationCesiumChunk *chunk = (WhirlyKitElevationCesiumChunk *)elevChunk.chunkImpl;

                           MaplyScreenLabel *label = [[MaplyScreenLabel alloc] init];
                           label.loc = center;
                           label.text = [NSString stringWithFormat:@"(%d,%d)=%lu",tileID.x,tileID.y,chunk.mesh->pts.size()];
                           MaplyComponentObject *compObj4 = [layer.viewC addScreenLabels:@[label] desc:
                                                             @{kMaplyFont: [UIFont systemFontOfSize:18.0],
                                                               kMaplyJustify: @"center",
                                                               kMaplyTextOutlineSize: @(1.0)
                                                               }];
                           
                           [layer addData:@[compObj0,compObj1,compObj2,compObj3,compObj4] forTile:tileID];
                           
                           [layer tileDidLoad:tileID];
                       }
                   });
}

@end

