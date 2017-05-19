//
//  VectorsTestCase.h
//  AutoTester
//
//  Created by jmnavarro on 29/10/15.
//  Copyright © 2015-2017 mousebird consulting. All rights reserved.
//

#import "MaplyTestCase.h"
#import "GeographyClassTestCase.h"

@class MaplyVectorObject;

@interface VectorsTestCase : MaplyTestCase

@property (nonatomic, strong) NSMutableArray<MaplyVectorObject*> *compList;
@property (nonatomic) GeographyClassTestCase *baseView;
@end
