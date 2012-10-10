/*
 *  MaplyInteractionLayer_private.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/19/12.
 *  Copyright 2011-2012 mousebird consulting
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
#import <set>
#import <WhirlyGlobe.h>
#import "MaplyComponentObject_private.h"
#import "SelectObject_private.h"

// The view controller fills this in
@protocol MaplyInteractionLayerDelegate <NSObject>
// Called back on the main thread for selection
- (void)handleSelection:(MaplyTapMessage *)msg didSelect:(NSObject *)selectedObj;
@end

/** The Interaction Layer runs in the layer thread (mostly) and manages
    data added to the scene and various other layers.
 */
@interface MaplyInteractionLayer : NSObject<WhirlyKitLayer>
{
    WhirlyKitMarkerLayer * __weak markerLayer;
    WhirlyKitLabelLayer * __weak labelLayer;
    WhirlyKitVectorLayer * __weak vectorLayer;
    WhirlyKitSelectionLayer * __weak selectLayer;
    // Note: This is not a good idea
    UIView * __weak glView;
    
    // Component objects created for the user
    NSMutableArray *userObjects;
    
    // The view controller, for various callbacks
    NSObject<MaplyInteractionLayerDelegate> * __weak viewController;
    
    // Use to map IDs in the selection layer to objects the user passed in
    SelectObjectSet selectObjectSet;
}

@property (nonatomic,weak) WhirlyKitMarkerLayer * markerLayer;
@property (nonatomic,weak) WhirlyKitLabelLayer * labelLayer;
@property (nonatomic,weak) WhirlyKitVectorLayer * vectorLayer;
@property (nonatomic,weak) WhirlyKitSelectionLayer * selectLayer;
@property (nonatomic,weak) UIView * glView;
@property (nonatomic,weak) NSObject<MaplyInteractionLayerDelegate> * viewController;

// Add screen space (2D) markers
- (MaplyComponentObject *)addScreenMarkers:(NSArray *)markers desc:(NSDictionary *)desc;

// Add 3D markers
- (MaplyComponentObject *)addMarkers:(NSArray *)markers desc:(NSDictionary *)desc;

// Add screen space (2D) labels
- (MaplyComponentObject *)addScreenLabels:(NSArray *)labels desc:(NSDictionary *)desc;

// Add 3D labels
- (MaplyComponentObject *)addLabels:(NSArray *)labels desc:(NSDictionary *)desc;

// Add vectors
- (MaplyComponentObject *)addVectors:(NSArray *)vectors desc:(NSDictionary *)desc;

// Remove objects associated with the user object
- (void)removeObject:(MaplyComponentObject *)userObj;

// Remove objects associated with the user objects
- (void)removeObjects:(NSArray *)userObjs;

// Call this to process a tap with the selection layer
// It will call the given selector if there was no selection
- (void) userDidTap:(MaplyTapMessage *)msg;

@end
