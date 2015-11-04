//
//  ShapesTestCase.m
//  AutoTester
//
//  Created by jmnavarro on 3/11/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

#import "ShapesTestCase.h"
#import "MaplyShape.h"
#import "MaplyBaseViewController.h"
#import "MaplyGeomModel.h"
#import "MaplyMatrix.h"
#import "GeographyClassTestCase.h"
#import "MaplyViewController.h"


// Simple representation of locations and name for testing
typedef struct
{
	char name[20];
	float lat,lon;
} LocationInfo;

// Some random locations for testing.
// If we've missed your home, it's because we think you suck.
static const int NumLocations = 30;
LocationInfo locations[NumLocations] =
{
	{"Kansas City",39.1, -94.58},
	{"Washington, DC",38.895111,-77.036667},
	{"Manila",14.583333,120.966667},
	{"Moscow",55.75, 37.616667},
	{"London",51.507222, -0.1275},
	{"Caracas",10.5, -66.916667},
	{"Lagos",6.453056, 3.395833},
	{"Sydney",-33.859972, 151.211111},
	{"Seattle",47.609722, -122.333056},
	{"Tokyo",35.689506, 139.6917},
	{"McMurdo Station",-77.85, 166.666667},
	{"Tehran",35.696111, 51.423056},
	{"Santiago",-33.45, -70.666667},
	{"Pretoria",-25.746111, 28.188056},
	{"Perth",-31.952222, 115.858889},
	{"Beijing",39.913889, 116.391667},
	{"New Delhi",28.613889, 77.208889},
	{"San Francisco",37.7793, -122.4192},
	{"Pittsburgh",40.441667, -80},
	{"Freetown",8.484444, -13.234444},
	{"Windhoek",-22.57, 17.083611},
	{"Buenos Aires",-34.6, -58.383333},
	{"Zhengzhou",34.766667, 113.65},
	{"Bergen",60.389444, 5.33},
	{"Glasgow",55.858, -4.259},
	{"Bogota",4.598056, -74.075833},
	{"Haifa",32.816667, 34.983333},
	{"Puerto Williams",-54.933333, -67.616667},
	{"Panama City",8.983333, -79.516667},
	{"Niihau",21.9, -160.166667}
};

@implementation ShapesTestCase

- (instancetype)init
{
	if (self = [super init]) {
		self.captureDelay = 5;
		self.name = @"Shapes";
	}
	return self;
}
static const float EarthRadius = 6371000;

// Add arrows
- (void)addArrows:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset desc:(NSDictionary *)desc baseViewC: (MaplyBaseViewController*) baseViewC
{
	// Start out the arrow at 1m
	double size = 1;
	double arrowCoords[2*7] = {-0.25*size,-0.75*size, -0.25*size,0.25*size, -0.5*size,0.25*size, 0.0*size,1.0*size,  0.5*size,0.25*size, 0.25*size,0.25*size, 0.25*size,-0.75*size};
	
	MaplyShapeExtruded *exShape = [[MaplyShapeExtruded alloc] initWithOutline:arrowCoords numCoordPairs:7];
	exShape.thickness = size * 1.0;
	exShape.height = 0.0;
	exShape.color = [UIColor colorWithRed:0.8 green:0.25 blue:0.25 alpha:1.0];
	// Each shape is about 10km
	//    exShape.transform = [[MaplyMatrix alloc] initWithScale:10000*1/EarthRadius];
	exShape.scale = 1.0;
	MaplyGeomModel *shapeModel = [[MaplyGeomModel alloc] initWithShape:exShape];
	
	NSMutableArray *arrows = [[NSMutableArray alloc] init];
	for (unsigned int ii=offset;ii<len;ii+=stride) {
		LocationInfo *loc = &locations[ii];
		MaplyGeomModelInstance *geomInst = [[MaplyGeomModelInstance alloc] init];
		MaplyCoordinate coord = MaplyCoordinateMakeWithDegrees(loc->lon, loc->lat);
		geomInst.center = MaplyCoordinate3dMake(coord.x, coord.y, 10000);
		MaplyMatrix *orientMat = [[MaplyMatrix alloc] initWithYaw:0.0 pitch:0.0 roll:45.0/180.0*M_PI];
		geomInst.transform = [[[MaplyMatrix alloc] initWithScale:10000*1/EarthRadius] multiplyWith:orientMat];
		geomInst.selectable = true;
		geomInst.model = shapeModel;
		
		[arrows addObject:geomInst];
	}
	
	[baseViewC addModelInstances:arrows desc:desc mode:MaplyThreadAny];
}

- (void)addShapeCylinders:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset desc:(NSDictionary *)desc baseViewC: (MaplyBaseViewController*) baseViewC
{
	NSMutableArray *cyls = [[NSMutableArray alloc] init];
	for (unsigned int ii=offset;ii<len;ii+=stride) {
		LocationInfo *location = &locations[ii];
		MaplyShapeCylinder *cyl = [[MaplyShapeCylinder alloc] init];
		cyl.baseCenter = MaplyCoordinateMakeWithDegrees(location->lon, location->lat);
		cyl.radius = 0.01;
		cyl.height = 0.06;
		cyl.selectable = true;
		[cyls addObject:cyl];
	}
	
	[baseViewC addShapes:cyls desc:desc];
}

- (void)addGreatCircles:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset desc:(NSDictionary *)desc baseViewC: (MaplyBaseViewController*) baseViewC
{
	NSMutableArray *circles = [[NSMutableArray alloc] init];
	for (unsigned int ii=offset;ii<len;ii+=stride) {
		LocationInfo *loc0 = &locations[ii];
		LocationInfo *loc1 = &locations[(ii+1)%len];
		MaplyShapeGreatCircle *greatCircle = [[MaplyShapeGreatCircle alloc] init];
		greatCircle.startPt = MaplyCoordinateMakeWithDegrees(loc0->lon, loc0->lat);
		greatCircle.endPt = MaplyCoordinateMakeWithDegrees(loc1->lon, loc1->lat);
		greatCircle.lineWidth = 6.0;
		greatCircle.selectable = true;
		// This limits the height based on the length of the great circle
		float angle = [greatCircle calcAngleBetween];
		greatCircle.height = 0.3 * angle / M_PI;
		[circles addObject:greatCircle];
	}
	
	[baseViewC addShapes:circles desc:desc ];
}

- (void)addShapeSpheres:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset desc:(NSDictionary *)desc baseViewC : (MaplyBaseViewController*) baseViewC
{
	NSMutableArray *spheres = [[NSMutableArray alloc] init];
	for (unsigned int ii=offset;ii<len;ii+=stride) {
		LocationInfo *location = &locations[ii];
		MaplyShapeSphere *sphere = [[MaplyShapeSphere alloc] init];
		sphere.center = MaplyCoordinateMakeWithDegrees(location->lon, location->lat);
		sphere.radius = 0.04;
		sphere.selectable = true;
		[spheres addObject:sphere];
	}
	
	[baseViewC addShapes:spheres desc:desc];
}


- (BOOL)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
	GeographyClassTestCase *baseLayer = [[GeographyClassTestCase alloc]init];
	[baseLayer setUpWithGlobe:globeVC];
	[self addArrows:locations len:NumLocations stride:4 offset:2 desc:@{kMaplyColor : [UIColor colorWithRed:1.0 green:0.1 blue:0.0 alpha:1.0], kMaplyFade: @(1.0)} baseViewC:(MaplyBaseViewController*)globeVC];
	[self addShapeCylinders:locations len:NumLocations stride:4 offset:0 desc:@{kMaplyColor : [UIColor colorWithRed:0.0 green:0.0 blue:1.0 alpha:0.8], kMaplyFade: @(1.0), kMaplyDrawPriority: @(1000)} baseViewC:(MaplyBaseViewController*)globeVC];
	[self addGreatCircles:locations len:NumLocations stride:4 offset:2 desc:@{kMaplyColor : [UIColor colorWithRed:1.0 green:0.1 blue:0.0 alpha:1.0], kMaplyFade: @(1.0), kMaplyDrawPriority: @(1000)} baseViewC:(MaplyBaseViewController*)globeVC];
	[self addShapeSpheres:locations len:NumLocations stride:4 offset:1 desc:@{kMaplyColor : [UIColor colorWithRed:1.0 green:0.0 blue:0.0 alpha:0.8], kMaplyFade: @(1.0), kMaplyDrawPriority: @(1000)} baseViewC:(MaplyBaseViewController *)globeVC];
	
	return true;
}


- (BOOL)setUpWithMap:(MaplyViewController *)mapVC
{
	GeographyClassTestCase *baseLayer = [[GeographyClassTestCase alloc]init];
	[baseLayer setUpWithMap:mapVC];
	[self addArrows:locations len:NumLocations stride:4 offset:2 desc:@{kMaplyColor : [UIColor colorWithRed:1.0 green:0.1 blue:0.0 alpha:1.0], kMaplyFade: @(1.0)} baseViewC:(MaplyBaseViewController*)mapVC];
	[self addShapeCylinders:locations len:NumLocations stride:4 offset:0 desc:@{kMaplyColor : [UIColor colorWithRed:0.0 green:0.0 blue:1.0 alpha:0.8], kMaplyFade: @(1.0), kMaplyDrawPriority: @(1000)} baseViewC:(MaplyBaseViewController*)mapVC];
	[self addGreatCircles:locations len:NumLocations stride:4 offset:2 desc:@{kMaplyColor : [UIColor colorWithRed:1.0 green:0.1 blue:0.0 alpha:1.0], kMaplyFade: @(1.0), kMaplyDrawPriority: @(1000)} baseViewC:(MaplyBaseViewController*)mapVC];
	[self addShapeSpheres:locations len:NumLocations stride:4 offset:1 desc:@{kMaplyColor : [UIColor colorWithRed:1.0 green:0.0 blue:0.0 alpha:0.8], kMaplyFade: @(1.0), kMaplyDrawPriority: @(1000)} baseViewC:(MaplyBaseViewController *)mapVC];
	return true;
}

@end
