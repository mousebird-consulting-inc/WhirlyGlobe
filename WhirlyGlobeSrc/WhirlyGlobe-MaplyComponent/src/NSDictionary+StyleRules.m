//
//  NSDictionary+StyleRules.m
//  WhirlyGlobe-MaplyComponent
//
//  Created by Jesse Crocker on 4/9/14.
//
//

#import "NSDictionary+StyleRules.h"

@implementation NSMutableDictionary (StyleRules)
- (NSMutableArray*)styles {
  NSMutableArray *styles = self[@"styles"];
  if(!styles) {
    styles = [NSMutableArray array];
    self[@"styles"] = styles;
  }
  return styles;
}

- (NSMutableArray*)rules {
  NSMutableArray *rules = self[@"rules"];
  if(!rules) {
    rules = [NSMutableArray array];
    self[@"rules"] = rules;
  }
  return rules;
}

- (NSMutableArray*)symbolizers {
  NSMutableArray *symbolizers = self[@"symbolizers"];
  if(!symbolizers) {
    symbolizers = [NSMutableArray array];
    self[@"symbolizers"] = symbolizers;
  }
  return symbolizers;
}

- (NSMutableArray*)layers {
  NSMutableArray *layers = self[@"layers"];
  if(!layers) {
    layers = [NSMutableArray array];
    self[@"layers"] = layers;
  }
  return layers;
}


- (NSString*)filter {
  return self[@"filter"];
}

- (void)setFilter:(NSString*)filter {
  self[@"filter"] = filter;
}

- (NSString*)name {
  return self[@"name"];
}

- (NSNumber*)minScaleDenom {
  return self[@"minscaledenom"];
}

- (void)setMinScaleDenom:(NSNumber*)num {
  self[@"minscaledenom"] = num;
}

- (NSNumber*)maxScaleDenom {
  return self[@"maxscaledenom"];
}

- (void)setMaxScaleDenom:(NSNumber*)num {
  self[@"maxscaledenom"] = num;
}

- (NSMutableDictionary*)parameters {
  NSMutableDictionary *params = self[@"parameters"];
  if(!params) {
    params = [NSMutableDictionary dictionary];
    self[@"parameters"] = params;
  }
  return params;
}

@end

void NSDictionaryStyleDummyFunc()
{
}