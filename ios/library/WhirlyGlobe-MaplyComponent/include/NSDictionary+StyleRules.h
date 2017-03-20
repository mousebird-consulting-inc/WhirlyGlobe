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
- (NSMutableDictionary*)parameters;

@end

// A function we can call to force the linker to bring in categories
#ifdef __cplusplus
extern "C"
#endif
void NSDictionaryStyleDummyFunc();
