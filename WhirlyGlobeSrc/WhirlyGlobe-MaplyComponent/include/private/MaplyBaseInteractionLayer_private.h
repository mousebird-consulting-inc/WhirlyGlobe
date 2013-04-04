/*
 *  MaplyBaseInteractionLayer_private.h
 *  MaplyComponent
 *
 *  Created by Steve Gifford on 12/14/12.
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

#import <Foundation/Foundation.h>
#import <set>
#import <WhirlyGlobe.h>
#import "MaplyComponentObject_private.h"
#import "SelectObject_private.h"
#import "ImageTexture_private.h"

@interface MaplyBaseInteractionLayer : NSObject<WhirlyKitLayer>
{
@public
    WhirlyKitMarkerLayer * __weak markerLayer;
    WhirlyKitLabelLayer * __weak labelLayer;
    WhirlyKitVectorLayer * __weak vectorLayer;
    WhirlyKitShapeLayer * __weak shapeLayer;
    WhirlyKitSphericalChunkLayer * __weak chunkLayer;
    WhirlyKitLoftLayer * __weak loftLayer;
    WhirlyKitSelectionLayer * __weak selectLayer;
    // Note: Not a great idea to be passing this in
    UIView * __weak glView;

    WhirlyKitView * __weak visualView;

    // Use to map IDs in the selection layer to objects the user passed in
    SelectObjectSet selectObjectSet;

    // Layer thread we're part of
    WhirlyKitLayerThread * __weak layerThread;

    // Scene we're using
    WhirlyKit::Scene *scene;
    
    // Used to track textures
    MaplyImageTextureSet imageTextures;

    // Component objects created for the user
    NSMutableArray *userObjects;
}

@property (nonatomic,weak) WhirlyKitMarkerLayer * markerLayer;
@property (nonatomic,weak) WhirlyKitLabelLayer * labelLayer;
@property (nonatomic,weak) WhirlyKitVectorLayer * vectorLayer;
@property (nonatomic,weak) WhirlyKitShapeLayer * shapeLayer;
@property (nonatomic,weak) WhirlyKitSphericalChunkLayer *chunkLayer;
@property (nonatomic,weak) WhirlyKitLoftLayer *loftLayer;
@property (nonatomic,weak) WhirlyKitSelectionLayer * selectLayer;
@property (nonatomic,weak) UIView * glView;

// Initialize with the view we'll be using
- (id)initWithView:(WhirlyKitView *)visualView;

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

// Add vectors that we'll only use for selection
- (MaplyComponentObject *)addSelectionVectors:(NSArray *)vectors desc:(NSDictionary *)desc;

// Change vector representation
- (void)changeVectors:(MaplyComponentObject *)vecObj desc:(NSDictionary *)desc;

// Add shapes
- (MaplyComponentObject *)addShapes:(NSArray *)shapes desc:(NSDictionary *)desc;

// Add stickers
- (MaplyComponentObject *)addStickers:(NSArray *)stickers desc:(NSDictionary *)desc;

// Add lofted polys
- (MaplyComponentObject *)addLoftedPolys:(NSArray *)vectors desc:(NSDictionary *)desc key:(NSString *)key cache:(NSObject<WhirlyKitLoftedPolyCache> *)cache;

// Remove objects associated with the user object
- (void)removeObject:(MaplyComponentObject *)userObj;

// Remove objects associated with the user objects
- (void)removeObjects:(NSArray *)userObjs;

///// Internal routines.  Don't ever call these outside of the layer thread.

// An internal routine to add an image to our local UIImage/ID cache
- (WhirlyKit::SimpleIdentity)addImage:(UIImage *)image;

// Remove the texture associated with an image  or just decrement its reference count
- (void)removeImage:(UIImage *)image;

// Do a point in poly check for vectors we're representing
- (NSObject *)findVectorInPoint:(WhirlyKit::Point2f)pt;

@end
