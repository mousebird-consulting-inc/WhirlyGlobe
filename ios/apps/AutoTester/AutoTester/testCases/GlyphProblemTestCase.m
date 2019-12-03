//
//  GlyphProblemTestCase.h
//  AutoTester
//
//  Created by zhoujiong on 2019/11/12.
//  Copyright © 2019 mousebird consulting. All rights reserved.
//

#define MIN_HEIGHT 0.000251845893
#define MAX_HEIGHT 2.0

#import "GlyphProblemTestCase.h"
#import "AutoTester-Swift.h"

@interface GlyphProblemTestCase()

@property (nonatomic, strong) GeographyClassTestCase *testCase;

@property (nonatomic, assign) BOOL isChenge;

@property (nonatomic,strong) NSMutableArray *maplyComponentObjectArray;

@end

@implementation GlyphProblemTestCase

- (instancetype)init
{
    if (self = [super init]) {
        self.name = @"GlyphProblem";
        self.implementations = MaplyTestCaseImplementationGlobe;
        
    }
    
    return self;
}

- (void) setup {
    // set up the data source
    self.testCase = [[GeographyClassTestCase alloc] init];
}

- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC{
    
    [self setup];
    [_testCase setUpWithGlobe:globeVC];
    
    [globeVC setZoomLimitsMin:MIN_HEIGHT max:MAX_HEIGHT];
    globeVC.keepNorthUp = true;//保持正北朝上
    globeVC.height = 2.0;
    globeVC.delegate = self;
    
    [globeVC setPosition:MaplyCoordinateMakeWithDegrees(120,40)];
}

- (void)globeViewController:(WhirlyGlobeViewController *)viewC didStopMoving:(MaplyCoordinate *)corners userMotion:(bool)userMotion{
    
    if (!self.maplyComponentObjectArray) {
        self.maplyComponentObjectArray = [NSMutableArray array];
    }
    NSArray *colorArray = @[[UIColor redColor],[UIColor blackColor],[UIColor greenColor],[UIColor brownColor]];

    int countX = 20;
    int countY = 20;
    
    for (int count = 0; count < 4; count ++) {
        
        NSMutableArray *labelArray = [NSMutableArray array];
        for (int i = 0; i < countX; i ++) {
            for (int j = 0; j < countY; j ++) {
                
                MaplyScreenLabel *annotationLabel = [[MaplyScreenLabel alloc] init];
                if (_isChenge) {
                    annotationLabel.text = [NSString stringWithFormat:@"你好Hello%d%d%d",i,j,count];
                }else{
                    annotationLabel.text = [NSString stringWithFormat:@"谢谢Thanks%d%d%d",i,j,count];
                }
                annotationLabel.layoutImportance = MAXFLOAT;
                annotationLabel.loc = MaplyCoordinateMakeWithDegrees(110 + i*0.4 + count*0.1, 30 + j*0.4  + count*0.1);
                [labelArray addObject:annotationLabel];
            }
        }
        UIColor *color = colorArray[count];
        if (self.maplyComponentObjectArray.count == 4) {
            MaplyComponentObject *obj = self.maplyComponentObjectArray[count];
            [viewC removeObject:obj];
            MaplyComponentObject *compObjPointLabel = [viewC addScreenLabels:labelArray desc:@{kMaplyFont: [UIFont systemFontOfSize:15],kMaplyTextColor:color,kMaplyTextOutlineSize : @(1.5),kMaplyTextOutlineColor : [UIColor whiteColor]} mode:MaplyThreadAny];
            self.maplyComponentObjectArray[count] = compObjPointLabel;
        }else{
            MaplyComponentObject *compObjPointLabel = [viewC addScreenLabels:labelArray desc:@{kMaplyFont: [UIFont systemFontOfSize:15],kMaplyTextColor:color,kMaplyTextOutlineSize : @(1.5),kMaplyTextOutlineColor : [UIColor whiteColor]} mode:MaplyThreadAny];
            [self.maplyComponentObjectArray addObject:compObjPointLabel];
        }
    }
    
    _isChenge = !_isChenge;
    
}

@end
