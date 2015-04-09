/*
 *  MapnikStyleRule.m
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Jesse Crocker, Trailbehind inc. on 3/31/14.
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


#import "MapnikStyleRule.h"
#import "MaplyVectorStyle.h"

@interface MapnikStyleRule ()
@property (nonatomic, strong, readwrite) NSMutableArray *symbolizers;
@end

@implementation MapnikStyleRule

- (instancetype) init {
  self = [super init];
  if(self) {
    self.symbolizers = [NSMutableArray new];
  }
  return self;
}


- (void)setFilter:(NSString*)filterExpression {
  NSMutableString *mutableFilterExpression = [NSMutableString stringWithString:filterExpression];
  [mutableFilterExpression replaceOccurrencesOfString:@"["
                                           withString:@""
                                              options:0
                                                range:NSMakeRange(0, mutableFilterExpression.length)];
  [mutableFilterExpression replaceOccurrencesOfString:@"]"
                                           withString:@""
                                              options:0
                                                range:NSMakeRange(0, mutableFilterExpression.length)];
  [mutableFilterExpression replaceOccurrencesOfString:@"mapnik::" //NSPredicate doesn't like :
                                           withString:@""
                                              options:0
                                                range:NSMakeRange(0, mutableFilterExpression.length)];
  [mutableFilterExpression replaceOccurrencesOfString:@"%" //remove format strings
                                           withString:@""
                                              options:0
                                                range:NSMakeRange(0, mutableFilterExpression.length)];
  @try {
    self.filterPredicate = [NSPredicate predicateWithFormat:mutableFilterExpression];
  }
  @catch (NSException *exception) {
    NSLog(@"Error parsing filter expression:%@", exception);
    self.filterPredicate = [NSPredicate predicateWithValue:NO];
  }
}


- (void)setMaxScaleDenomitator:(NSUInteger)maxScaleDenomitator {
  _maxScaleDenomitator = maxScaleDenomitator;
  self.minZoom = [MapnikStyleRule scaleToZoom:maxScaleDenomitator];
}


- (void)setMinScaleDenominator:(NSUInteger)minScaleDenominator {
  _minScaleDenominator = minScaleDenominator;
  self.maxZoom = [MapnikStyleRule scaleToZoom:minScaleDenominator];
}


+ (NSUInteger)scaleToZoom:(NSUInteger)scale {
  NSUInteger zoom;
  double testScale = 279541132.014;
  for(zoom = 1; testScale > scale; zoom++) {
    testScale = testScale / 2.0;
  }
  return zoom;
}


- (NSString*)description {
  return [NSString stringWithFormat:@"%lu - %lu(%lu - %lu): %@, %lu symbolizers",
          (unsigned long)self.minScaleDenominator, (unsigned long)self.maxScaleDenomitator,
          (unsigned long)self.minZoom, (unsigned long)self.maxZoom, self.filterPredicate,
          (unsigned long)self.symbolizers.count];
}


@end
