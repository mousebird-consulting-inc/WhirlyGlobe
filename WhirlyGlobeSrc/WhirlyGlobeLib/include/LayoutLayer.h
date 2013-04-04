/*
 *  LayoutLayer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 12/4/12.
 *  Copyright 2011-2012 mousebird consulting. All rights reserved.
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

#import <math.h>
#import <set>
#import <map>
#import "Identifiable.h"
#import "Drawable.h"
#import "DataLayer.h"
#import "LayerThread.h"

/// Okay to place to the right of a point
#define WhirlyKitLayoutPlacementRight  (1<<0)
/// Okay to place it to the left of a point
#define WhirlyKitLayoutPlacementLeft   (1<<1)
/// Okay to place on top of a point
#define WhirlyKitLayoutPlacementAbove  (1<<2)
/// Okay to place below a point
#define WhirlyKitLayoutPlacementBelow  (1<<3)

/** This represents an object in the screen space generator to be laid ouit
    by the layout engine.  We'll manipulate its offset and enable/disable it
    but won't otherwise change it.
 */
@interface WhirlyKitLayoutObject : NSObject
{
@public
    /// Object as represented by the screen space generator
    WhirlyKit::SimpleIdentity ssID;
    /// Any other objects we want to enable or disable in connection with this one.
    /// Think map icon.
    WhirlyKit::SimpleIDSet auxIDs;
    /// Location in display coordinate system
    WhirlyKit::Point3f dispLoc;
    /// Size (in pixels) of the object we're laying out
    WhirlyKit::Point2f size;
    /// If we're hovering around an icon, this is its size in pixels.  Zero means its just us.
    WhirlyKit::Point2f iconSize;
    /// Minimum visiblity
    float minVis;
    /// Maximum visibility
    float maxVis;
    /// This is used to sort objects for layout.  Bigger is more important.
    float importance;
    /// Options for where to place this object:  WhirlyKitLayoutPlacementLeft, WhirlyKitLayoutPlacementRight,
    ///  WhirlyKitLayoutPlacementAbove, WhirlyKitLayoutPlacementBelow
    int acceptablePlacement;
    /// Used for debugging
    NSString *tag;
}

@end

/** The layout layer is a 2D text and marker layout engine.  You feed it objects
    you want it to draw and it will route them accordingly and control their
    visibility as new objects are added or old ones removed.
  */
@interface WhirlyKitLayoutLayer : NSObject<WhirlyKitLayer>

/// If set to a value greater than zero, that will the max number of objects displayed.
/// Zero by default, meaning all visible objects will be displayed (that can fit).
@property (nonatomic,assign) int maxDisplayObjects;

/// Initialize with the renderer (for screen size)
- (id)initWithRenderer:(WhirlyKitSceneRendererES *)renderer;

/// Called in the layer thread
- (void)startWithThread:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene;

/// Called in the layer thread
- (void)shutdown;

/// Add a whole bunch of objects to track with the layout engine.
- (void)addLayoutObjects:(NSArray *)layoutObjects;

/// Stop tracking a bunch of objects.  We assume they're removed elsewhere.
- (void)removeLayoutObjects:(const WhirlyKit::SimpleIDSet &)objectIDs;

@end
