//
//  SLDSymbolizers.h
//  SLDTest
//
//  Created by Ranen Ghosh on 2016-08-12.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "DDXML.h"
#import "MaplyVectorTileStyle.h"

@interface SLDSymbolizer : NSObject
@property (nonatomic, strong) MaplyVectorTileStyle * _Nonnull maplyVectorTileStyle;
+ (BOOL)matchesSymbolizerNamed:(NSString * _Nonnull)symbolizerName;
+ (NSArray<MaplyVectorTileStyle *> *) maplyVectorTileStyleWithElement:(DDXMLElement * _Nonnull)element tileStyleSettings:(MaplyVectorStyleSettings *)tileStyleSettings viewC:(MaplyBaseViewController *)viewC;
@end

@interface SLDLineSymbolizer : SLDSymbolizer
@end

@interface SLDPolygonSymbolizer : SLDSymbolizer
@end

@interface SLDPointSymbolizer : SLDSymbolizer
@end

@interface SLDTextSymbolizer : SLDSymbolizer
@end



