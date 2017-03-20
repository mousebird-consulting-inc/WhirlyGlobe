//
//  MaplyVectorStyleSimple.h
//  WhirlyGlobe-MaplyComponent
//
//  Created by Steve Gifford on 3/15/16.
//
//

#import <Foundation/Foundation.h>
#import "MapboxVectorTiles.h"

/** @brief Simple default style to see something in vector tile data.
    @details A simple vector style that displays each layer in a random color.
    Use this as a starting point for your own style.
  */
@interface MaplyVectorStyleSimpleGenerator : NSObject<MaplyVectorStyleDelegate>

@property (nonatomic,weak) MaplyBaseViewController *viewC;

/// @brief Initialize with a map view controller
- (id)initWithViewC:(MaplyBaseViewController *)viewC;

@end

/** @brief Base class for the simple vector style.
  */
@interface MaplyVectorStyleSimple : NSObject<MaplyVectorStyle>

/// @brief Unique Identifier for this style
@property (nonatomic,strong) NSString *uuid;

/// @brief Set if this geometry is additive (e.g. sticks around) rather than replacement
@property (nonatomic) bool geomAdditive;

/// @brief Priority for sorting among layers
@property (nonatomic) int drawPriority;

@property (nonatomic,weak) MaplyBaseViewController *viewC;

- (id)initWithViewC:(MaplyBaseViewController *)viewC;

@end

/** @brief Simple filled polygon with a random color.
  */
@interface MaplyVectorStyleSimplePolygon : MaplyVectorStyleSimple

@property (nonatomic,strong) UIColor *color;

@end

/** @brief Simple point we'll convert to a label.
  */
@interface MaplyVectorStyleSimplePoint : MaplyVectorStyleSimple

@property (nonatomic,strong) UIFont *font;

@end

/** @brief Simple linear with a random color.
  */
@interface MaplyVectorStyleSimpleLinear : MaplyVectorStyleSimple

@property (nonatomic,strong) UIColor *color;

@end
