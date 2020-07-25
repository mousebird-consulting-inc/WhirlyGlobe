/*
 *  MaplyIconManager.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/11/14.
 *  Copyright 2011-2019 mousebird consulting
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

#import <UIKit/UIKit.h>
#import "control/MaplyRenderController.h"

/**
 Holds info about a single style from the MaplySimpleStyleManager.
 This is enough to build a marker (or other thing, if you like).
 */
@interface MaplySimpleStyle : NSObject

/// If there was a title, this is it
@property (nonatomic,nullable) NSString *title;

/// If there was a description, this is it
@property (nonatomic,nullable) NSString *desc;

/// Size (in pixels) of the marker to be built
@property (nonatomic) CGSize markerSize;

/// How big we consider the marker to be when doing layout.
/// By default, same as the marker size
@property (nonatomic) CGSize layoutSize;

/// Offset applied to marker
@property (nonatomic) CGPoint markerOffset;

/// Texture constructed for this icon, if there was a symbol
@property (nonatomic,nullable) MaplyTexture *markerTex;

/// If this was 0-9 or a-Z instead, this is that
@property (nonatomic,nullable) NSString *markerString;

/// Color to set for the markert
@property (nonatomic,nonnull) UIColor *color;

/// Stroke color if there is one
@property (nonatomic,nonnull) UIColor *strokeColor;

/// Stroke opacity
@property (nonatomic) float strokeOpacity;

/// Stroke width, if available.  Takes scale into account.
@property (nonatomic) float strokeWidth;

/// Fill color if available
@property (nonatomic,nonnull) UIColor *fillColor;

/// Fill opacity, if available
@property (nonatomic) float fillOpacity;

@end

/** Used to generate icons and parse styles for the GeoJSON simple Style spec.
    https://github.com/mapbox/simplestyle-spec
 
    Can also be used to define some very simple icon styles directly.
 */
@interface MaplySimpleStyleManager : NSObject

/** Fetch the simple UIImage for the icon with the given name.
  **/
+ (nullable UIImage *)iconForName:(NSString *__nonnull)name size:(CGSize)size;

/** Slightly more complex icon
  **/
+ (nullable UIImage *)iconForName:(NSString *__nullable)name size:(CGSize)size color:(UIColor *__nullable)color circleColor:(UIColor *__nullable)circleColor strokeSize:(float)strokeSize strokeColor:(UIColor *__nullable)strokeColor;

/**
 Set up the icon manager this way to build textures associated with a particular view controller.
 */
- (nonnull id)initWithViewC:(NSObject<MaplyRenderControllerProtocol> * __nonnull)viewC;

/// Markers can be three different sizes.  These are the actual sizes associated
@property (nonatomic) CGSize smallSize,medSize,largeSize;

/// Normal scale from device (e.g. 2x for retina and so on)
@property (nonatomic) CGFloat scale;

/// We normally put a stroke around generated icons
/// This is the width (in pixels) of that stroek
@property (nonatomic) CGFloat strokeWidthForIcons;

/// If set (default) we'll center the marker.  If off we'll offset vertically
@property (nonatomic) bool centerIcon;

/**
 Mapbox defines a simple style spec that's usually associated with GeoJSON data.  Github is a prominent user.

 Pass in a dictionary parsed from JSON (or just make it up yourself) and this will produce (an optional) icon and parse out the rest.
 This takes screen scale and such into account.  It will also cache the same description when passed in multiple times.
*/
- (MaplySimpleStyle * __nonnull)makeStyle:(NSDictionary *__nonnull)dict;

/**
 Takes a single vector object.  It will parse out the simple style from the attributes (or provide a default if there is none)
   and then build the corresponding feature and return a MaplyComponentObject to represent it.
 
 mode controls if this work is done on this thread or another.
 */
- (MaplyComponentObject * __nullable)addFeature:(MaplyVectorObject * __nonnull)vecObj mode:(MaplyThreadMode)mode;

/**
  Takes an array of vector objects and calls addFeature: on each one.

 mode controls if this work is done on this thread or another.
 */
- (NSArray<MaplyComponentObject *> * __nonnull)addFeatures:(NSArray<MaplyVectorObject *> * __nonnull)vecObjs mode:(MaplyThreadMode)mode;

/// Delete any cached textures and such
- (void)shutdown;

@end
