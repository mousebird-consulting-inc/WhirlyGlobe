/*
 *  MapboxVectorStyleSet.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/16/15.
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

#import <Foundation/Foundation.h>
#import "vector_styles/MaplyVectorStyle.h"
#import "vector_tiles/MapboxVectorTiles.h"

typedef NS_ENUM(NSUInteger,MapboxLayerType) {
    MapboxLayerTypeBackground,
    MapboxLayerTypeCircle,
    MapboxLayerTypeFill,
    MapboxLayerTypeLine,
    MapboxLayerTypeRaster,
    MapboxLayerTypeSymbol,
    MapboxLayerTypeUnknown
};

/**
 A single entry in the legend array returned by
 */
@interface MaplyLegendEntry : NSObject

/// Name of this entry
@property (nonatomic,nonnull) NSString *name;

/// Image for this entry, if this is a single entry
@property (nonatomic,nullable) UIImage *image;

/// Array of entries if this is a group
@property (nonatomic,nullable) NSMutableArray<MaplyLegendEntry *> *entries;

@end

/** @brief The Mapbox Vector Style Set parses Mapbox GL Style sheets and turns them into Maply compatible styles.
    @details A style delegate is required by the Mapnik parser to build geometry out of Mapnik vector tiles.  This style delegate can read a Mapbox GL Style sheet and produce compatible styles.
 */
@interface MapboxVectorStyleSet : NSObject<MaplyVectorStyleDelegate>

/// @brief Initialize with the style dictionary alreayd parsed from JSON
/// @details We'll parse the style JSON passed in and return nil on failure.
/// @details The optional filter can be used to reject layers we won't use
- (id __nullable)initWithDict:(NSDictionary * __nonnull)styleDict
                     settings:(MaplyVectorStyleSettings * __nonnull)settings
                        viewC:(NSObject<MaplyRenderControllerProtocol> * __nonnull)viewC;

/// @brief Initialize with the style JSON and the view controller
/// @details We'll parse the style JSON passed in and return nil on failure.
/// @details The optional filter can be used to reject layers we won't use
- (id __nullable)initWithJSON:(NSData * __nonnull)styleJSON
                     settings:(MaplyVectorStyleSettings * __nonnull)settings
                        viewC:(NSObject<MaplyRenderControllerProtocol> * __nonnull)viewC;

/// @brief Where we can fetch the sprites
@property (nonatomic, strong, nullable) NSString *spriteURL;

/// Tile sources
@property (nonatomic, strong, nonnull) NSArray *sources;

/// All the layer names
@property (nonatomic) NSArray<NSString *> * __nonnull layerNames;

/// Type of the given layer
- (MapboxLayerType) layerType:(NSString * __nonnull)layerName;

/// Add the sprint sheet for use in symbols.  Return false on failures.
- (bool)addSprites:(NSDictionary * __nonnull)spriteDict image:(UIImage * __nonnull)image;

/**
 This method will poke around in the given layer to determine a distinc color for it.
 For circle layers, you get the circle color.  For fill and line layers, it's the paint color.
 For symbols, you get the text color.
 This is useful for visualizing layers, it has nothing to do with rendering them.
 */
- (UIColor * __nullable) colorForLayer:(NSString *__nonnull)layerName;

/// If there is a background layer, calculate the color for a given zoom level.
/// Otherwise return nil
- (UIColor * __nullable)backgroundColorForZoom:(double)zoom;

/// Make a layer visible/invisible
- (void)setLayerVisible:(NSString *__nonnull)layerName visible:(bool)visible;

/**
 Returns a dictionary containing a flexible legend for the layers contained in this style.
 Each layer is rendered as a representative image at the given size.
 Layer names that start with the same "<name>_" will be grouped together in the hiearchy if
  the group parameter is set.  Otherwise they'll be flat.
 */
- (NSArray<MaplyLegendEntry *> * __nonnull)layerLegend:(CGSize)imageSize group:(bool)useGroups;

@property (nonatomic, weak, nullable) NSObject<MaplyRenderControllerProtocol> *viewC;

@end

typedef enum : NSUInteger {
    MapboxSourceVector,
    MapboxSourceRaster,
    // TODO: Support the rest of these eventually
} MapboxSourceType;

// Sources are called out individually
@interface MaplyMapboxVectorStyleSource : NSObject

// Name of the source
@property (nonatomic,nullable) NSString *name;

// Vector and raster sources supported for now
@property (nonatomic) MapboxSourceType type;

// TileJSON URL, if present
@property (nonatomic,nullable) NSString *url;

// If the TileJSON spec is inline, this is it
@property (nonatomic,nullable) NSDictionary *tileSpec;

// Initialize with the entry in the style file
- (id __nullable)initWithName:(NSString *__nonnull)name styleEntry:(NSDictionary * __nonnull)styleEntry styleSet:(MapboxVectorStyleSet * __nonnull)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> * __nonnull)viewC;

@end
