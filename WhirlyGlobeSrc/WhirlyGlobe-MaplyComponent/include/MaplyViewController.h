/*
 *  MaplyViewController.h
 *  MaplyComponent
 *
 *  Created by Steve Gifford on 9/6/12.
 *  Copyright 2012 mousebird consulting
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
#import <MaplyCoordinate.h>
#import "MaplyScreenMarker.h"
#import "MaplyVectorObject.h"
#import "MaplyViewTracker.h"
#import "MaplyComponentObject.h"
#import "MaplySharedAttributes.h"

@class MaplyViewControllerLayer;
@class MaplyViewController;

/** Fill in this protocol for callbacks when the user taps on or near something.
  */
@protocol MaplyViewControllerDelegate <NSObject>

@optional

/// Called when the user taps on or near an object.
/// You're given the object you passed in originally, such as a MaplyScreenMarker
- (void)maplyViewController:(MaplyViewController *)viewC didSelect:(NSObject *)selectedObj;

/// User tapped at a given location.
/// This won't be called if they tapped and selected, just for taps.
- (void)maplyViewController:(MaplyViewController *)viewC didTapAt:(MaplyCoordinate)coord;

@end

/** The main object for the Maply Component.  Create a view controller, add it to
    your view hirarchy and start tossing in data.  You'll want a base layer at the
    very least and then you can add markers, labels, and vectors on top.
  */
@interface MaplyViewController : UIViewController
{
    NSObject<MaplyViewControllerDelegate> * __weak delegate;
}

/// Set this to trun on/off the pinch (zoom) gesture recognizer
/// On by default
@property(nonatomic,assign) bool pinchGesture;

/// Set this to turn on or off the rotation gesture recognizer.
/// On by default.
@property(nonatomic,assign) bool rotateGesture;

/// Set this to get callbacks for various events.
@property(nonatomic,weak) NSObject<MaplyViewControllerDelegate> *delegate;

/// Set selection support on or off here
@property(nonatomic,assign) bool selection;

/// Set the globe view's background color.
/// Black, by default.
@property (nonatomic,strong) UIColor *clearColor;

/// Get/set the current height above terrain.
/// The radius of the earth is 1.0.  Height above terrain is relative to that.
@property (nonatomic,assign) float height;

/// Add rendering and other general hints for the globe view controller.
- (void)setHints:(NSDictionary *)hintsDict;

/// Animate to the given position over the given amount of time
- (void)animateToPosition:(WGCoordinate)newPos time:(NSTimeInterval)howLong;

/// Set the view to the given position immediately
- (void)setPosition:(WGCoordinate)newPos;

/// Set position and height at the same time
- (void)setPosition:(WGCoordinate)newPos height:(float)height;


/// Add a quad tree paged earth layer with MapBox Tiles on top
- (MaplyViewControllerLayer *)addQuadEarthLayerWithMBTiles:(NSString *)name;

/// Add a quad tree paged earth layer with
- (MaplyViewControllerLayer *)addQuadEarthLayerWithRemoteSource:(NSString *)baseURL imageExt:(NSString *)ext cache:(NSString *)cachdDir minZoom:(int)minZoom maxZoom:(int)maxZoom;

/// Add visual defaults for the screen markers
- (void)setScreenMarkerDesc:(NSDictionary *)desc;

/// Add a group of screen (2D) markers
- (MaplyComponentObject *)addScreenMarkers:(NSArray *)markers;

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

/// Add visual defaults for the vectors
- (void)setVectorDesc:(NSDictionary *)desc;

/// Add one or more vectors
- (WGComponentObject *)addVectors:(NSArray *)vectors;

/// Add visual defaults for the shapes
- (void)setShapeDesc:(NSDictionary *)desc;

/// Add one or more shapes
- (WGComponentObject *)addShapes:(NSArray *)shapes;

/// Add a view to track to a particular location
- (void)addViewTracker:(WGViewTracker *)viewTrack;

/// Remove the view tracker associated with the given UIView
- (void)removeViewTrackForView:(UIView *)view;

/// Remove the data associated with an object the user added earlier
- (void)removeObject:(WGComponentObject *)theObj;

/// Remove an array of data objects
- (void)removeObjects:(NSArray *)theObjs;

@end
