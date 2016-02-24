//
//  RunwayBuilderTestCase.m
//  AutoTester
//
//  Created by Steve Gifford on 1/28/16.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

#import "RunwayBuilderTestCase.h"
#import "AutoTester-swift.h"

@implementation RunwayBuilderTestCase

- (instancetype)init
{
    if (self = [super init]) {
        // Note: Debugging
        self.captureDelay = 100000;
        self.name = @"Runway Builder";
    }
    return self;
}

// Make a black texture with a line around it
- (UIImage *)colorOutlineTexture:(UIColor *)color
{
    CGSize size;  size = CGSizeMake(16,128);
    UIGraphicsBeginImageContext(size);
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    
    [color setFill];
    CGContextFillRect(ctx, CGRectMake(0, 0, size.width, size.height));
    
    [[UIColor blackColor] setFill];
    CGContextFillRect(ctx, CGRectMake(2, 2, size.width-4, size.height-4));

    UIImage *retImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();

    return retImage;
}

// Build a chevron straddling the origin
- (MaplyGeomBuilder *)buildChevron:(double)width thick:(double)thick state:(MaplyGeomState *)state viewC:(MaplyBaseViewController *)viewC
{
    MaplyGeomBuilder *geomBuilder = [[MaplyGeomBuilder alloc] initWithViewC:viewC];
    
    double width2 = width/2.0;
 
    // Left side
    [geomBuilder addCurPointX:-width2 y:0.0];
    [geomBuilder addCurPointX:0.0 y:width2];
    [geomBuilder addCurPointX:0.0 y:width2+thick];
    [geomBuilder addCurPointX:-width2 y:thick];
    [geomBuilder addCurPoly:state];

    // Right side
    [geomBuilder addCurPointX:width2 y:0.0];
    [geomBuilder addCurPointX:width2 y:thick];
    [geomBuilder addCurPointX:0.0 y:width2+thick];
    [geomBuilder addCurPointX:0.0 y:width2];
    [geomBuilder addCurPoly:state];
    
    return geomBuilder;
}

// This is a simple fake runway, intended as a test.
// This is not a real runway
- (MaplyGeomModel *)buildRunwayModel:(MaplyBaseViewController *)viewC
{
    UIImage *tarmacImage = [self colorOutlineTexture:[UIColor whiteColor]];
    UIImage *grayImage = [self colorOutlineTexture:[UIColor grayColor]];

    // Color states
    MaplyGeomState *blankState = [[MaplyGeomState alloc] init];
    blankState.color = [UIColor blackColor];
    MaplyGeomState *textState = [[MaplyGeomState alloc] init];
    textState.color = [UIColor whiteColor];
    textState.backColor = [UIColor blackColor];
    MaplyGeomState *tarmacState = [[MaplyGeomState alloc] init];
    tarmacState.color = [UIColor whiteColor];
    tarmacState.texture = tarmacImage;
    MaplyGeomState *grayOutlineState = [[MaplyGeomState alloc] init];
    grayOutlineState.color = [UIColor whiteColor];
    grayOutlineState.texture = grayImage;
    MaplyGeomState *stripeState = [[MaplyGeomState alloc] init];
    stripeState.color = [UIColor whiteColor];
    MaplyGeomState *yellowStripeState = [[MaplyGeomState alloc] init];
    yellowStripeState.color = [UIColor yellowColor];
    
    MaplyGeomBuilder *wholeBuilder = [[MaplyGeomBuilder alloc] initWithViewC:viewC];
    
    // Various size constants
    double width = 80.0;
    double overrun = 200.0;
    double displaced = 200.0;
    double runway = 2000.0;
    double chevronThickness = 4.0;
    double drawOffset = 10.0;
    enum {BaseLayer,PaintLayer};
    double total = overrun+displaced+runway;
    
    // Overrun on the end
    {
        MaplyGeomBuilder *geomBuilder = [[MaplyGeomBuilder alloc] initWithViewC:viewC];
        [geomBuilder addRectangleAroundX:0.0 y:overrun/2.0 width:width height:overrun state:blankState];

        // Toss in a couple of chevrons
        MaplyGeomBuilder *chevronBuilder = [self buildChevron:width thick:chevronThickness state:yellowStripeState viewC:viewC];
        [geomBuilder addGeomFromBuilder:chevronBuilder transform:[[MaplyMatrix alloc] initWithTranslateX:0.0 y:0.0 z:PaintLayer*drawOffset]];
        [geomBuilder addGeomFromBuilder:chevronBuilder transform:[[MaplyMatrix alloc] initWithTranslateX:0.0 y:width/2.0+2*chevronThickness z:PaintLayer*drawOffset]];
        
        [wholeBuilder addGeomFromBuilder:geomBuilder transform:[[MaplyMatrix alloc] initWithTranslateX:0.0 y:0.0 z:0.0]];
    }

    // Next up, the displaced area
    {
        MaplyGeomBuilder *geomBuilder = [[MaplyGeomBuilder alloc] initWithViewC:viewC];
        [geomBuilder addRectangleAroundX:0.0 y:displaced/2.0 width:width height:displaced state:grayOutlineState];
        
        [wholeBuilder addGeomFromBuilder:geomBuilder transform:[[MaplyMatrix alloc] initWithTranslateX:0.0 y:overrun z:0.0]];
    }
    
    // Main part of the runway
    {
        MaplyGeomBuilder *geomBuilder = [[MaplyGeomBuilder alloc] initWithViewC:viewC];
        [geomBuilder addRectangleAroundX:0.0 y:runway/2.0 width:width height:runway state:tarmacState];
        
        // Numbers
        MaplyGeomBuilder *numBuilder = [[MaplyGeomBuilder alloc] initWithViewC:viewC];
        UIFont *font = [UIFont boldSystemFontOfSize:128.0];
        [numBuilder addString:@"09" width:0.66*width height:0.0 font:font state:textState];
        MaplyCoordinate3dD size = [numBuilder getSize];
        [geomBuilder addGeomFromBuilder:numBuilder transform:[[MaplyMatrix alloc] initWithTranslateX:-size.x/2.0 y:width z:PaintLayer*drawOffset]];
        
        // Letter
        MaplyGeomBuilder *letterBuilder = [[MaplyGeomBuilder alloc] initWithViewC:viewC];
        [letterBuilder addString:@"R" width:0.0 height:size.y font:font state:textState];
        MaplyCoordinate3dD letterSize = [letterBuilder getSize];
        [geomBuilder addGeomFromBuilder:letterBuilder transform:[[MaplyMatrix alloc] initWithTranslateX:-letterSize.x/2.0 y:width/2.0 z:PaintLayer*drawOffset]];

        [wholeBuilder addGeomFromBuilder:geomBuilder transform:[[MaplyMatrix alloc] initWithTranslateX:0.0 y:(overrun+displaced) z:0.0]];
    }
    
    // Balance it in the middle and then tilt it
    double runwayMinZ = 100.0;
    double runwayMaxZ = 110.0;
    double rot = asin((runwayMaxZ-runwayMinZ)/total);
    [wholeBuilder rotate:rot aroundX:1.0 y:0.0 z:0.0];
    [wholeBuilder translateX:0.0 y:0.0 z:(runwayMinZ+runwayMaxZ)/2.0];
    
    return [wholeBuilder makeGeomModel:MaplyThreadCurrent];
}

- (BOOL)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC {
    NSString *cacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)  objectAtIndex:0];
    NSString *jsonTileSpec = @"http://a.tiles.mapbox.com/v3/examples.map-zyt2v9k2.json";
    NSString *thisCacheDir = [NSString stringWithFormat:@"%@/mbtilessat1/",cacheDir];

    NSURLRequest *request = [NSURLRequest requestWithURL:[NSURL URLWithString:jsonTileSpec]];
    NSURLSession *session = [NSURLSession sharedSession];
    NSURLSessionDataTask *task = [session dataTaskWithRequest:request completionHandler:
    ^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {
        NSError *jsonError;
        NSDictionary *tileSourceDict;
        if (!error)
            tileSourceDict = [NSJSONSerialization JSONObjectWithData:data options:kNilOptions error:&jsonError];
        if (!error && !jsonError) {
            MaplyRemoteTileSource *tileSource = [[MaplyRemoteTileSource alloc] initWithTilespec:tileSourceDict];
            tileSource.cacheDir = thisCacheDir;
            
            MaplyQuadImageTilesLayer *layer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:tileSource.coordSys tileSource:tileSource];
            layer.handleEdges = true;
            [globeVC addLayer:layer];
            
        } else {
            NSLog(@"Failed to reach JSON tile spec at: %@",jsonTileSpec);
        }
    }];
    [task resume];
    
    // Build the model
    MaplyGeomModel *geomModel = [self buildRunwayModel:globeVC];
    
    // Use the model
    MaplyGeomModelInstance *modelInst = [[MaplyGeomModelInstance alloc] init];
    modelInst.model = geomModel;
    MaplyCoordinate coord = MaplyCoordinateMakeWithDegrees(-122.270833, 37.804444);
    MaplyMatrix *headingMat = [[MaplyMatrix alloc] initWithAngle:M_PI/4.0 axisX:0 axisY:0 axisZ:1.0];
    modelInst.transform = [headingMat multiplyWith:[[MaplyMatrix alloc] initWithScale:1/6371000.0]];
    modelInst.center = MaplyCoordinate3dMake(coord.x,coord.y,200.0);
    
    [globeVC addModelInstances:@[modelInst] desc:@{kMaplyDrawPriority: @(1000000),kMaplyZBufferWrite: @(YES), kMaplyZBufferRead: @(YES)} mode:MaplyThreadCurrent];
    
    [globeVC setPosition:coord];
    globeVC.height = 0.001;
    globeVC.keepNorthUp = true;
    globeVC.clearColor = [UIColor grayColor];
    return true;
}


@end
