//
//  NSDictionary+StyleRules.h
//  WhirlyGlobe-MaplyComponent
//
//  Created by Jesse Crocker on 4/9/14.
//
//

#import <Foundation/Foundation.h>

@interface NSMutableDictionary (StyleRules)

- (NSMutableArray*)styles;
- (NSMutableArray*)rules;
- (NSMutableArray*)symbolizers;
- (NSMutableArray*)layers;
- (NSString*)filter;
- (void)setFilter:(NSString*)filter;
- (NSString*)name;

- (NSNumber*)minScaleDenom;
- (void)setMinScaleDenom:(NSNumber*)num;
- (NSNumber*)maxScaleDenom;
- (void)setMaxScaleDenom:(NSNumber*)num;

@end
