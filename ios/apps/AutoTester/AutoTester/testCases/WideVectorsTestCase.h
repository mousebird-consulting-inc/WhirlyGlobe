//
//  WideVectorsTestCase.h
//  AutoTester
//
//  Created by jmnavarro on 3/11/15.
//  Copyright 2015-2022 mousebird consulting.
//

#import "MaplyTestCase.h"

@interface WideVectorsTestCaseBase : MaplyTestCase

- (instancetype)initWithName:(NSString*)name supporting:(MaplyTestCaseImplementations)impl;

- (void) loadShapeFile: (MaplyBaseViewController*) baseViewC;

- (NSArray *)addGeoJson:(NSString*)name
            dashPattern:(NSArray*)dashPattern
                  width:(CGFloat)width
                   edge:(double)edge
                 simple:(bool)simple
                  viewC:(MaplyBaseViewController *)baseViewC;

- (NSArray *)addGeoJson:(NSString*)name
            dashPattern:(NSArray*)dashPattern
                  width:(CGFloat)width
                  viewC:(MaplyBaseViewController *)baseViewC;

- (NSArray *)addGeoJson:(NSString*)name viewC:(MaplyBaseViewController *)viewC;

- (NSArray *)addWideVectors:(MaplyVectorObject *)vecObj
                  baseViewC:(MaplyBaseViewController*)baseViewC
              dashedLineTex:(MaplyTexture*)dashedLineTex
              filledLineTex:(MaplyTexture*)filledLineTex;

- (void)vecColors:(MaplyBaseViewController *)viewC;

- (void)overlap:(MaplyBaseViewController *)viewC;

@end
