/*
 *  GlobeViewController.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/21/12.
 *  Copyright 2011 mousebird consulting
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
#import <WGCoordinate.h>
#import <WGScreenMarker.h>

@class WGViewControllerLayer;
@class WGComponentObject;
@class WhirlyGlobeViewController;

/// Use this hint to turn the zbuffer on or off.  Pass in an NSNumber boolean.  Takes effect on the next frame.
#define kWGRenderHintZBuffer @"zbuffer"
/// Use this hint to turn culling optimization on or off.  Pass in an NSNumber boolean.
#define kWGRenderHintCulling @"culling"

/// These are used for all object descriptions.

/// If the z buffer is on, this will let you resolve.  Takes an NSNumber boolean
#define kWGDrawOffset @"drawOffset"
/// This helps decide what order things are drawn in.  Useful when the z buffer is off or you're using transparency.
/// Takes an NSNumber int.
#define kWGDrawPriority @"drawPriority"
/// Minimum point at which a feature is visible.  Takes an NSNumber float.  The radius of the globe is 1.0
#define kWGMinVis @"minVis"
/// Maximum point at which a feature is visible.  Takes an NSNumber float.  The radius of the globe is 1.0
#define kWGMaxVis @"maxVis"
/// The amount of time for a feature to fade in or out.  Takes an NSNumber float for seconds.
#define kWGFade @"fade"

/// Default draw offset for 3D markers.  Set to avoid label/marker intererence
#define kWGMarkerDrawOffsetDefault 1
#define kWGMarkerDrawPriorityDefault 1

/// These are used just for the screen and regular labels

/// Color of the text being rendered.  Takes a UIColor.
#define kWGTextColor @"textColor"
/// Background color for the text.  Takes a UIColor.
#define kWGBackgroundColor @"backgroundColor"
/// Font to use in rendering text.  Takes a UIFont.
#define kWGFont @"font"
/// Default height of the text.  If for screen space, this in points.  If for 3D, remember that
//   the radius of the globe is 1.0.  Expects an NSNumber float.
#define kWGLabelHeight @"height"
/// Default width of the text.  See height for more info and, in general, use height instead.
#define kWGLabelWidth @"width"
/// Justification for label placement.  This takes an NSString with one of:
///  middle, left, right
#define kWGJustify @"justify"

/// Default draw offset for 3D labels.  This is set to avoid label/marker interference
#define kWGLabelDrawOffsetDefault 2
#define kWGLabelDrawPriorityDefault 2

/// These are used for screen and regular markers.

/// Color is used for the polygon generated for a marker.  It will combine with the image,
///  if there is one or it will be visible if there is no texture.  Takes a UIColor
#define kWGColor @"color"


/** Fill in this protocol to get callbed when the user taps on or near an object for
    selection. 
 */
@protocol WhirlyGlobeViewControllerDelegate <NSObject>

@optional
/// Called when the user taps on or near an object.
/// You're given the object you passed in original, such as a WGScreenMarker.
- (void)globeViewController:(WhirlyGlobeViewController *)viewC didSelect:(NSObject *)selectedObj;

/// Called when the user taps outside the globe.
/// Passes in the location on the view.
- (void)globeViewControllerDidTapOutside:(WhirlyGlobeViewController *)viewC;

@end

/** This is the main object in the WhirlyGlobe Component.  You fire up one
    of these, add its view to your view hierarchy and start tossing in data.
    At the very lead you'll want a base image layer and then you can put
    markers, labels, and vectors on top of that.
 */
@interface WhirlyGlobeViewController : UIViewController
{
    NSObject<WhirlyGlobeViewControllerDelegate> *delegate;
}

/// Set this to keep the north pole facing upward when moving around.
/// Off by default.
@property(nonatomic,assign) bool keepNorthUp;

/// Set this to get callbacks for various events.
@property(nonatomic,strong) NSObject<WhirlyGlobeViewControllerDelegate> *delegate;

/// Set selection support on or off here
@property(nonatomic,assign) bool selection;

/// Add rendering and other general hints for the globe view controller.
- (void)setHints:(NSDictionary *)hintsDict;

/// Animate to the given position over the given amount of time
- (void)animateToPosition:(WGCoordinate)newPos time:(NSTimeInterval)howLong;

/// Set the view to the given position immediately
- (void)setPosition:(WGCoordinate)newPos;

/// Add a spherical earth layer with the given set of base images
- (WGViewControllerLayer *)addSphericalEarthLayerWithImageSet:(NSString *)name;

/// Add a quad tree paged earth layer with MapBox Tiles on top
- (WGViewControllerLayer *)addQuadEarthLayerWithMBTiles:(NSString *)name;

/// Add a quad tree paged earth layer with 
- (WGViewControllerLayer *)addQuadEarthLayerWithRemoteSource:(NSString *)baseURL imageExt:(NSString *)ext cache:(NSString *)cachdDir minZoom:(int)minZoom maxZoom:(int)maxZoom;

/// Add visual defaults for the screen markers
- (void)setScreenMarkerDesc:(NSDictionary *)desc;

/// Add a group of screen (2D) markers
- (WGComponentObject *)addScreenMarkers:(NSArray *)markers;

/// Add visual defaults for the markers
- (void)setMarkerDesc:(NSDictionary *)desc;

/// Add a group of 3D markers
- (WGComponentObject *)addMarkers:(NSArray *)markers;

/// Add visual defaults for the screen labels
- (void)setScreenLabelDesc:(NSDictionary *)desc;

/// Add a group of screen (2D) labels
- (WGComponentObject *)addScreenLabels:(NSArray *)labels;

/// Add visual defaults for the labels
- (void)setLabelDesc:(NSDictionary *)desc;

/// Add a group of 3D labels
- (WGComponentObject *)addLabels:(NSArray *)labels;

/// Remove the data associated with an object the user added earlier
- (void)removeObject:(WGComponentObject *)theObj;

@end
