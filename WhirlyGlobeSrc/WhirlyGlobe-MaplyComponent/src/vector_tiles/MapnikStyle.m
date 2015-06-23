/*
 *  MapnikStyle.mm
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

#import "MapnikStyle.h"
#import "MaplyVectorStyle.h"

@interface MapnikStyle ()

@property (nonatomic, strong, readwrite) NSMutableArray *rules;

@end


@implementation MapnikStyle


- (instancetype)init {
  self = [super init];
  if(self) {
    self.rules = [NSMutableArray new];
    self.filterModeFirst = NO;
    self.opacity = 1.0;
  }
  return self;
}


- (void)addRule:(MapnikStyleRule *)rule {
  [self.rules addObject:rule];
}


- (NSString*)description {
  return [NSString stringWithFormat:@"name:%@ filterMode:%@ opacity:%f rules:%lu", self.name,
          self.filterModeFirst?@"first":@"all", self.opacity, (unsigned long)self.rules.count];
}


- (NSString*)debugDescription {
  NSMutableString *string = [NSMutableString stringWithFormat:@"MapnikStyle name:%@ filterMode:%@ opacity:%f rules:%lu", self.name,
                             self.filterModeFirst?@"first":@"all", self.opacity, (unsigned long)self.rules.count];
  for(MapnikStyleRule *rule in self.rules) {
    [string appendFormat:@"\n    %@", rule.description];
  }
  return string;
}


@end
