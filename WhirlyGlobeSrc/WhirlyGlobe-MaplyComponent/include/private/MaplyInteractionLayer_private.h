/*
 *  MaplyInteractionLayer_private.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/19/12.
 *  Copyright 2011-2013 mousebird consulting
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
#import "MaplyBaseInteractionLayer_private.h"

<<<<<<< HEAD
//// The view controller fills this in
@protocol MaplyInteractionLayerDelegate <NSObject>
// Called back on the main thread for selection
// Note: Porting
//- (void)handleSelection:(MaplyTapMessage *)msg didSelect:(NSObject *)selectedObj;
=======
// The view controller fills this in
@protocol MaplyInteractionLayerDelegate <NSObject>
// Called back on the main thread for selection
- (void)handleSelection:(MaplyTapMessage *)msg didSelect:(NSObject *)selectedObj;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
@end

/** The Interaction Layer runs in the layer thread (mostly) and manages
    data added to the scene and various other layers.
 */
@interface MaplyInteractionLayer : MaplyBaseInteractionLayer

<<<<<<< HEAD
// Note: Porting
/// The view controller, for various callbacks
//@property (nonatomic,weak) NSObject<MaplyInteractionLayerDelegate> * viewController;
@property (nonatomic,weak) NSObject* viewController;

// Create with the map view
- (id)initWithMapView:(Maply::MapView *)inMapView;

// Note: Porting
//// Call this to process a tap with the selection layer
//// It will call the given selector if there was no selection
//- (void) userDidTap:(MaplyTapMessage *)msg;
=======
/// The view controller, for various callbacks
@property (nonatomic,weak) NSObject<MaplyInteractionLayerDelegate> * viewController;

// Create with the map view
- (id)initWithMapView:(MaplyView *)inMapView;

// Call this to process a tap with the selection layer
// It will call the given selector if there was no selection
- (void) userDidTap:(MaplyTapMessage *)msg;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b

@end
