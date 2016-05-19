//
//  VectorsTestCase.h
//  AutoTester
//
//  Created by jmnavarro on 29/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

#import "MaplyTestCase.h"
#import "GeographyClassTestCase.h"

@interface VectorsTestCase : MaplyTestCase

@property (nonatomic, strong) NSMutableArray * compList;
@property (nonatomic) GeographyClassTestCase *baseView;
@end
