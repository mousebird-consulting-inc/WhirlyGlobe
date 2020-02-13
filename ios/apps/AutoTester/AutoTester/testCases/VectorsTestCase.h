//
//  VectorsTestCase.h
//  AutoTester
//
//  Created by jmnavarro on 29/10/15.
//  Copyright © 2015-2017 mousebird consulting.
//

#import "MaplyTestCase.h"

@class MaplyVectorObject;
@class GeographyClassTestCase;

@interface VectorsTestCase : MaplyTestCase

@property (nonatomic, strong) NSMutableArray<MaplyVectorObject*> *vecList;
@property (nonatomic, strong) NSMutableArray<MaplyComponentObject*> *compObjs;
@property (nonatomic) GeographyClassTestCase *baseView;
@end
