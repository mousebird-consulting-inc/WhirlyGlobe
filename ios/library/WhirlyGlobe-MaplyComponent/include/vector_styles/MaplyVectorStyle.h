/*  MaplyVectorStyle.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/3/14.
 *  Copyright 2011-2021 mousebird consulting
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
 */

#import <UIKit/UIKit.h>
#import "MaplyTileSourceNew.h"
#import "MaplyBaseViewController.h"

@class MaplyVectorTileData;

/** 
    Settings that control how vector tiles look in relation to their styles.
    
    These are set based on the sort of device we're on, particularly retina vs. non-retina.  They can be manipulated directly as well for your needs.
  */
@interface MaplyVectorStyleSettings : NSObject

/// Initialize with the (presumably) retina scale
- (instancetype __nonnull)initWithScale:(CGFloat)scale;

/// Line widths will be scaled by this amount before display.
@property (nonatomic) float lineScale;
/// Text sizes will be scaled by this amount before display.
@property (nonatomic) float textScale;
/// Markers (symbols+circles) will be scaled by this amount before display.
@property (nonatomic) float markerScale;
/// Symbols will be scaled by this amount before display.
@property (nonatomic) float symbolScale;
/// Circles will be scaled by this amount before display.
@property (nonatomic) float circleScale;
/// Importance for markers in the layout engine
@property (nonatomic) float markerImportance;
/// Default marker size when none is specified
@property (nonatomic) float markerSize;
/// Importance for labels in the layout engine
@property (nonatomic) float labelImportance;
/// If set we'll use the zoom levels defined in the style
@property (nonatomic) bool useZoomLevels;

/// For symbols we'll try to pull a UUID out of this field to stick in the marker and label uniqueID
@property (nonatomic,nullable) NSString *uuidField;

/// Draw priority calculated as offset from here
@property (nonatomic) int baseDrawPriority;

/// Offset between levels
@property (nonatomic) int drawPriorityPerLevel;

/** 
    The overall map scale calculations will be scaled by this amount.
    
    We use the map scale calculations to figure out what is dispalyed and when.  Not what to load in, mind you, that's a separate, but related calculation.  This controls the scaling of those calculations.  Scale it down to load things in later, up to load them in sooner.
  */
@property (nonatomic) float mapScaleScale;

/// Dashed lines will be scaled by this amount before display.
@property (nonatomic) float dashPatternScale;

/// Use widened vectors (which do anti-aliasing and such)
@property (nonatomic) bool useWideVectors;

/// Where we're using old vectors (e.g. not wide) scale them by this amount
@property (nonatomic) float oldVecWidthScale;

/// If we're using widened vectors, only activate them for strokes wider than this.  Defaults to zero.
@property (nonatomic) float wideVecCuttoff;

/// If set, this is the shader we'll use on the areal features.
@property (nonatomic,strong) NSString * _Nullable arealShaderName;

/// If set, we'll make all the features selectable.  If not, we won't.
@property (nonatomic) bool selectable;

/// If set, icons will be loaded from this directory
@property (nonatomic, strong) NSString * _Nullable iconDirectory;

/// The default font family for all text
@property (nonatomic,strong) NSString * _Nullable fontName;

@end

@protocol MaplyVectorStyle;

/** 
    Protocol for styling the vectors.
 
    You pass in an object which adheres to this protocol and will style
 the vectors read by a MaplyMapnikVectorTiles object.  In general, this will be
 a parsed Mapnik vector file, but you can substitute your own logic as well.
 */
@protocol MaplyVectorStyleDelegate <NSObject>

/** 
    Return the styles that apply to the given feature (attributes).
 */
- (nullable NSArray *)stylesForFeatureWithAttributes:(NSDictionary *__nonnull)attributes
                                              onTile:(MaplyTileID)tileID
                                             inLayer:(NSString *__nonnull)layer
                                               viewC:(NSObject<MaplyRenderControllerProtocol> *__nonnull)viewC;

/// Return true if the given layer is meant to display for the given tile (zoom level)
- (BOOL)layerShouldDisplay:(NSString *__nonnull)layer tile:(MaplyTileID)tileID;

/// Return the style associated with the given UUID.
- (nullable NSObject<MaplyVectorStyle> *)styleForUUID:(long long)uiid viewC:(NSObject<MaplyRenderControllerProtocol> *__nonnull)viewC;

/// Return the style for the background
- (nullable NSObject<MaplyVectorStyle> *)backgroundStyleViewC:(NSObject<MaplyRenderControllerProtocol> *__nonnull)viewC;

// Return a list of all the styles in no particular order.  Needed for categories and indexing
- (NSArray * __nonnull)allStyles;

@end

/** 
    Base protocol for the vector styles.
    
    Maply Vector Style is the protocol the your vector style needs to
    implement for the vector tile parsers to recognize it.
  */
@protocol MaplyVectorStyle<NSObject>

/// Unique Identifier for this style
- (long long) uuid;

/// Category used for sorting
- (NSString * _Nullable) getCategory;

/// Set if this geometry is additive (e.g. sticks around) rather than replacement
- (bool) geomAdditive;

/// Construct objects related to this style based on the input data.
- (void)buildObjects:(NSArray * _Nonnull)vecObjs
             forTile:(MaplyVectorTileData * __nonnull)tileData
               viewC:(NSObject<MaplyRenderControllerProtocol> * _Nonnull)viewC
                desc:(NSDictionary * _Nullable)desc;

/// Construct objects related to this style based on the input data.
- (void)buildObjects:(NSArray * _Nonnull)vecObjs
             forTile:(MaplyVectorTileData * __nonnull)tileData
               viewC:(NSObject<MaplyRenderControllerProtocol> * _Nonnull)viewC
                desc:(NSDictionary * _Nullable)desc
            cancelFn:(bool(^__nullable)(void))cancelFn;

@end

#ifdef __cplusplus
extern "C" {
#endif
/** 
    Use a style delegate to interpret vector data.
 
    Run the style delegate against the given vectors.  The resulting features are added to the
 given view controller using the thread mode specified.
 
    @param vecObjs An array of MaplyVectorObject.
 
    @param styleDelegate The style delegate that controls how the vectors will look.
 
    @param viewC View controller to add the geometry to.
 
    @param threadMode MaplyThreadCurrent will block until all the features are added.  MaplyThreadAny will do some of the work on another thread.
 */
NSArray<MaplyComponentObject*> * _Nonnull AddMaplyVectorsUsingStyle(NSArray<MaplyVectorObject*> * _Nonnull vecObjs,
                                                                    NSObject<MaplyVectorStyleDelegate> * _Nonnull styleDelegate,
                                                                    NSObject<MaplyRenderControllerProtocol> * _Nonnull viewC,
                                                                    MaplyThreadMode threadMode);

/**
    Use a style delegate to interpret vector data.
 
    Run the style delegate against the given vectors.  The resulting features are added to the
 given view controller using the thread mode specified.
 
    @param vecObjs An array of MaplyVectorObject.
 
    @param styleDelegate The style delegate that controls how the vectors will look.
 
    @param viewC View controller to add the geometry to.

    @param tileId The tile where the feature originates.
 
    @param enable Automatically enable the generated objects
 
    @param threadMode MaplyThreadCurrent will block until all the features are added.  MaplyThreadAny will do some of the work on another thread.
 
    @param desc Additional attributes to include with the generated component objects
 */
NSArray<MaplyComponentObject*> * _Nonnull AddMaplyVectorsUsingStyleAndAttributes(
    NSArray<MaplyVectorObject*> * _Nonnull vecObjs,
    NSObject<MaplyVectorStyleDelegate> * _Nonnull styleDelegate,
    NSObject<MaplyRenderControllerProtocol> * _Nonnull viewC,
    MaplyTileID tileId,
    bool enable,
    MaplyThreadMode threadMode,
    NSDictionary * _Nullable desc);

#ifdef __cplusplus
}
#endif
