//
//  VectorHoleTestCase.m
//  AutoTester
//
//  Created by Steve Gifford on 8/11/16.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//

#import "VectorHoleTestCase.h"

@interface VectorHoleTestCase()

@property (nonatomic) MaplyBaseViewController *viewC;

@end


@implementation VectorHoleTestCase

- (instancetype)init
{
    if (self = [super init]) {
        self.name = @"Vectors with Holes";
        self.captureDelay = 5;
        self.implementations = MaplyTestCaseImplementationMap | MaplyTestCaseImplementationGlobe;
        
    }
    
    return self;
}


- (void) overlayVectors: (MaplyBaseViewController*) baseVC
{
    NSDictionary *vectorDict = @{kMaplyShader: kMaplyShaderDefaultLine,
                                 kMaplyColor: [UIColor redColor],
                                 kMaplySelectable: @(true),
                                 kMaplyFilled: @(YES),
                                 kMaplyVecWidth: @(4.0)};
    
#if 0
    //Opp A (Anti-clockwise)
       MaplyCoordinate coord[5] = {MaplyCoordinateMakeWithDegrees(-2, 27),MaplyCoordinateMakeWithDegrees(-35, 27),MaplyCoordinateMakeWithDegrees(-35, -12),MaplyCoordinateMakeWithDegrees(-2, -12),MaplyCoordinateMakeWithDegrees(-2, 27)}; // = (MaplyCoordinate *)[pol.polyline bytes];
    
    //Opp B (clockwise)
//    MaplyCoordinate coord[5] = {MaplyCoordinateMakeWithDegrees(-35,27 ),MaplyCoordinateMakeWithDegrees(-2, 27),MaplyCoordinateMakeWithDegrees(-2, -12),MaplyCoordinateMakeWithDegrees(-35, -12),MaplyCoordinateMakeWithDegrees(-35,27 )};
    
    
    MaplyVectorObject *mainVector =  [[MaplyVectorObject alloc] initWithAreal:coord numCoords:5 attributes:nil];
    
    // Op A (Anti-clockwise) HOLE
//    MaplyCoordinate coordHo[5] = {MaplyCoordinateMakeWithDegrees(-20, 5),MaplyCoordinateMakeWithDegrees(-20, -5),MaplyCoordinateMakeWithDegrees(-10, -5),MaplyCoordinateMakeWithDegrees(-10, 5),MaplyCoordinateMakeWithDegrees(-20, 5)};
    
    // Op B (clockwise) HOLE
  MaplyCoordinate coordHo[5] = {MaplyCoordinateMakeWithDegrees(-20, 5),MaplyCoordinateMakeWithDegrees(-10, 5),MaplyCoordinateMakeWithDegrees(-10, -5),MaplyCoordinateMakeWithDegrees(-20, -5),MaplyCoordinateMakeWithDegrees(-20, 5)};
    
    [mainVector addHole:coordHo numCoords:5];

//    [mainVector subdivideToGlobe:0.001];
    mainVector = [mainVector clipToGrid:CGSizeMake(10.0/180.0*M_PI,10.0/180.0*M_PI)];
    mainVector = [mainVector tesselate];
    
    [baseVC addVectors:@[mainVector] desc:vectorDict];
#endif
    
    // This test case is GeoJSON from a customer
    NSString *geoJSON = @"{\"type\":\"Polygon\",\"coordinates\":[[[-8.666667,27.333333],[-3.583333,24.166667],[-3.6,16.35],[-4.066667,14.516667],[-4.283333,12.866667],[-4.8,12],[-5.25,11.5],[-5.533333,10.366667],[-5.616667,10.033333],[-2.75,9.866667],[-2.733333,9.833333],[-2.783333,9.683333],[-2.7,9.516667],[-2.666667,9.266667],[-2.766667,9.083333],[-2.65,8.95],[-2.583333,8.783333],[-2.5,8.2],[-2.616667,8.116667],[-2.633333,8.033333],[-2.783333,7.9],[-2.899689,7.572889],[-2.933333,7.5],[-3.033333,7.066667],[-3.216667,6.766667],[-3.233333,6.5],[-3.182406,6.395247],[-3.016667,5.85],[-3.016667,5.7],[-2.95,5.7],[-2.916667,5.616667],[-2.85,5.65],[-2.766667,5.55],[-2.733333,5.116667],[-3.1,5.1],[-3,4.683333],[-3,-1.85],[-3,-2.85],[-3,-3.85],[-3,-4.85],[-3,-5.85],[-3,-6.85],[-3,-7.85],[-3,-8.85],[-3,-9.533333],[-10,-12],[-10,-11],[-10,-10],[-10,-9],[-10,-8],[-10,-7],[-10,-6.366667],[-11,-6.366667],[-12,-6.366667],[-13,-6.366667],[-14,-6.366667],[-15,-6.366667],[-16,-6.366667],[-35,7.666667],[-37.5,13.5],[-37.5,14.5],[-37.5,15.5],[-37.5,16.5],[-37.5,17],[-24.35,13.666667],[-21.366667,12.966667],[-20,15],[-20,16],[-20,17],[-20,18],[-20,19],[-20,20],[-19,19],[-17.066667,20.783333],[-17.033333,21.05],[-16.933333,21.333333],[-15.933333,21.333333],[-14.933333,21.333333],[-13.933333,21.333333],[-13,21.333333],[-13.166667,22.833333],[-13.1666669999992,22.8333329999994],[-13.1683588232566,22.8637819724651],[-13.1684362596771,22.8942711086124],[-13.1668980406684,22.9247268892993],[-13.1637467983819,22.9550758275552],[-13.1589890697992,22.985244646381],[-13.1526352913168,23.0151604565099],[-13.1446997837383,23.0447509336992],[-13.135200727607,23.0739444951139],[-13.1241601288375,23.1026704743656],[-13.1116037746286,23.130859294764],[-13.0975611796681,23.1584426403417],[-13.0820655226683,23.1853536242094],[-13.0651535732977,23.2115269538051],[-13.0468656096073,23.2368990925992],[-13.0272453260794,23.2614084178262],[-13.006339732457,23.2849953738175],[-12.9841990435453,23.307602620516],[-12.9608765602094,23.3291751767653],[-12.9364285418232,23.3496605579745],[-12.9109140704579,23.3690089077715],[-12.8843949071295,23.387173123272],[-12.8569353404595,23.4041089736071],[-12.8286020281283,23.419775211367],[-12.7994638315369,23.4341336766387],[-12.7695916441165,23.4471493933338],[-12.7390582137559,23.4587906575235],[-12.7079379598409,23.4690291175221],[-12.6763067854239,23.4778398454807],[-12.6442418850647,23.4852014002816],[-12.6118215489011,23.4910958815456],[-12.5791249635278,23.4955089745947],[-12.5462320102726,23.4984299862383],[-12.5132230614755,23.4998518712795],[-12.4999999999931,23.4999999999958],[-12.5,23.5],[-12,23.5],[-12,24.5],[-12,25.5],[-12,26],[-11,26],[-10,26],[-9,26],[-8.666667,26],[-8.666667,27],[-8.666667,27.333333]],[[-16.916667,9],[-7.333333,0.000003],[-7.45,6.25],[-8.416667,7.5],[-8,10.166667],[-8,11.208333],[-8.00000000000023,11.208333],[-8.01938131213908,11.2094223293755],[-8.0858297660406,11.2152373326007],[-8.15191407696065,11.2242499189187],[-8.21747705030586,11.2364387932745],[-8.28236267577561,11.2517750799512],[-8.34641648834695,11.2702223891148],[-8.40948592598516,11.2917369011795],[-8.4714206832846,11.3162674687785],[-8.53207306025851,11.3437557360776],[-8.59129830551375,11.3741362751254],[-8.64895495306525,11.40733673889],[-8.70490515206608,11.4432780305937],[-8.75901498875147,11.4818744889158],[-8.81115479991877,11.5230340886028],[-8.86119947728975,11.5666586559895],[-8.90902876212612,11.6126440989062],[-8.95452752949404,11.6608806504201],[-8.99758606159775,11.7112531258351],[-9.03810030962581,11.7636411923505],[-9.07597214357662,11.8179196507619],[-9.11110958955096,11.8739587285692],[-9.14342705402009,11.9316243838403],[-9.17284553459627,11.9907786191667],[-9.19929281685041,12.0512798050329],[-9.22270365673721,12.1129830119127],[-9.24301994820297,12.1757403503905],[-9.2601908755648,12.2394013186005],[-9.27417305026232,12.3038131562617],[-9.28493063159508,12.3688212045805],[-9.29243543107067,12.4342692712791],[-9.296667,12.5],[-9.296667,12.5],[-9.366667,12.433333],[-9.366667,12.3],[-9.4,12.25],[-9.666667,12.1],[-9.7,12.033333],[-10.333333,12.216667],[-10.516667,12.066667],[-10.683333,11.883333],[-10.9,12.2],[-11.05,12.216667],[-11.25,12],[-11.483333,12.2],[-11.366667,12.416667],[-12.05,12.416667],[-12.35,12.3],[-12.5,12.383333],[-12.55,12.366667],[-12.666667,12.433333],[-12.9,12.533333],[-13,12.466667],[-13.066667,12.516667],[-13.066667,12.633333],[-13.716667,12.666667],[-13.666667,12.483333],[-13.766667,12.25],[-13.8,12.283333],[-13.933333,12.233333],[-13.95,12.15],[-13.7,12],[-13.716667,11.7],[-13.85,11.75],[-13.916667,11.683333],[-14.133333,11.65],[-14.266667,11.666667],[-14.516667,11.5],[-14.57,11.508333],[-14.683333,11.483333],[-14.966667,10.966667],[-15.083333,10.883333],[-16.916667,9]]]}";
    NSData *geoJSONData = [geoJSON dataUsingEncoding:NSASCIIStringEncoding];
    MaplyVectorObject *vecObj = [MaplyVectorObject VectorObjectFromGeoJSONApple:geoJSONData];
    vecObj = [vecObj clipToGrid:CGSizeMake(10.0/180.0*M_PI,10.0/180.0*M_PI)];
    vecObj = [vecObj tesselate];
    [baseVC addVectors:@[vecObj] desc:vectorDict];
}

- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
    self.baseView = [[GeographyClassTestCase alloc]init];
    [self.baseView setUpWithGlobe:globeVC];
    self.viewC = globeVC;
    //Overlay Countries
    [self overlayVectors:(MaplyBaseViewController*)globeVC];
}


- (void)setUpWithMap:(MaplyViewController *)mapVC
{
    self.baseView = [[GeographyClassTestCase alloc]init];
    [self.baseView setUpWithMap:mapVC];
    self.viewC = mapVC;
    [self overlayVectors:(MaplyBaseViewController*)mapVC];
}

- (void)handleSelection:(id)selectedObjs
{
    [self.baseViewController clearAnnotations];
    
    bool isMultiple = [selectedObjs isKindOfClass:[NSArray class]] && [(NSArray *)selectedObjs count] >= 1;

	MaplyVectorObject *theVector = nil;

    if (isMultiple) {
        MaplySelectedObject *firstObj = [(NSArray *)selectedObjs objectAtIndex:0];
        if ([firstObj.selectedObj isKindOfClass:[MaplyVectorObject class]]) {
			theVector = (MaplyVectorObject *)firstObj.selectedObj;
        }
    }
    else if ([selectedObjs isKindOfClass:[MaplyVectorObject class]]) {
		theVector = (MaplyVectorObject *)selectedObjs;
    }

	if (theVector) {
		MaplyCoordinate location = [theVector center];
		MaplyAnnotation *annotate = [[MaplyAnnotation alloc]init];
		annotate.title = @"Selected";
		annotate.subTitle = (NSString *)theVector.userObject;
		[self.viewC addAnnotation:annotate forPoint:location offset:CGPointZero];
	}
}

@end
