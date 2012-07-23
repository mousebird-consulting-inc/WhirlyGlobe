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

/** Fill in this protocol to get callbed when the user taps on or near an object for
    selection. 
 */
@protocol WhirlyGlobeViewControllerDelegate <NSObject>

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

/// Animate to the given position over the given amount of time
- (void)animateToPosition:(WGCoordinate)newPos time:(NSTimeInterval)howLong;

/// Set the view to the given position immediately
- (void)setPosition:(WGCoordinate)newPos;

/// Add a spherical earth layer with the given set of base images
- (WGViewControllerLayer *)addSphericalEarthLayerWithImageSet:(NSString *)name;

/// Add a group of screen (2D) markers
- (WGComponentObject *)addScreenMarkers:(NSArray *)markers;

/// Remove the data associated with an object the user added earlier
- (void)removeObject:(WGComponentObject *)theObj;

@end
