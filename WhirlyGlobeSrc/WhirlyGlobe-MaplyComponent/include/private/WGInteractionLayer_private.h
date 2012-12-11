/*
 *  WGInteractionLayer_private.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/21/12.
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
@protocol WGInteractionLayerDelegate <NSObject>
// Called back on the main thread for selection
- (void)handleSelection:(WhirlyGlobeTapMessage *)msg didSelect:(NSObject *)selectedObj;
@end

/** The Interaction Layer runs in the layer thread (mostly) and manages
    data added to the scene and various other layers.
 */
@interface WGInteractionLayer : NSObject<WhirlyKitLayer>
{
    WhirlyKitMarkerLayer * __weak markerLayer;
    WhirlyKitLabelLayer * __weak labelLayer;
    WhirlyKitVectorLayer * __weak vectorLayer;
    WhirlyKitShapeLayer * __weak shapeLayer;
    WhirlyKitSphericalChunkLayer * __weak chunkLayer;
    WhirlyKitSelectionLayer * __weak selectLayer;
    WhirlyGlobeView * __weak globeView;
    // Note: This is not a good idea
    UIView * __weak glView;
    
    // Component objects created for the user
    NSMutableArray *userObjects;
    
    // The view controller, for various callbacks
    NSObject<WGInteractionLayerDelegate> * __weak viewController;
    
    // Use to map IDs in the selection layer to objects the user passed in
    SelectObjectSet selectObjectSet;
    
    // If set, we'll autorotate after a certain amount of time
    float autoRotateInterval,autoRotateDegrees;
    
    // Last time something was tapped
    NSTimeInterval lastTouched;
}

@property (nonatomic,weak) WhirlyKitMarkerLayer * markerLayer;
@property (nonatomic,weak) WhirlyKitLabelLayer * labelLayer;
@property (nonatomic,weak) WhirlyKitVectorLayer * vectorLayer;
@property (nonatomic,weak) WhirlyKitShapeLayer * shapeLayer;
@property (nonatomic,weak) WhirlyKitSphericalChunkLayer *chunkLayer;
@property (nonatomic,weak) WhirlyKitSelectionLayer * selectLayer;
@property (nonatomic,weak) UIView * glView;
@property (nonatomic,weak) NSObject<WGInteractionLayerDelegate> * viewController;

// Initialize with the globeView
-(id)initWithGlobeView:(WhirlyGlobeView *)globeView;

// Set the autorotate values
- (void)setAutoRotateInterval:(float)autoRotateInterval degrees:(float)autoRotateDegrees;

// Add screen space (2D) markers
- (WGComponentObject *)addScreenMarkers:(NSArray *)markers desc:(NSDictionary *)desc;

// Add 3D markers
- (WGComponentObject *)addMarkers:(NSArray *)markers desc:(NSDictionary *)desc;

// Add screen space (2D) labels
- (WGComponentObject *)addScreenLabels:(NSArray *)labels desc:(NSDictionary *)desc;

// Add 3D labels
- (WGComponentObject *)addLabels:(NSArray *)labels desc:(NSDictionary *)desc;

// Add vectors
- (WGComponentObject *)addVectors:(NSArray *)vectors desc:(NSDictionary *)desc;

// Add vectors that we'll only use for selection
- (WGComponentObject *)addSelectionVectors:(NSArray *)vectors desc:(NSDictionary *)desc;

// Change vector representation
- (void)changeVectors:(WGComponentObject *)vecObj desc:(NSDictionary *)desc;

// Add shapes
- (WGComponentObject *)addShapes:(NSArray *)shapes desc:(NSDictionary *)desc;

// Add stickers
- (WGComponentObject *)addStickers:(NSArray *)stickers desc:(NSDictionary *)desc;

// Remove objects associated with the user object
- (void)removeObject:(WGComponentObject *)userObj;

// Remove objects associated with the user objects
- (void)removeObjects:(NSArray *)userObjs;

// Call this to process a tap with the selection layer
// It will call the given selector if there was no selection
- (void) userDidTap:(WhirlyGlobeTapMessage *)msg;

@end
