/*  MaplyInteractionLayer_private.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/21/12.
 *  Copyright 2011-2022 mousebird consulting
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

#import "MaplyInteractionLayer_private.h"
#import "visual_objects/MaplyScreenMarker.h"
#import "visual_objects/MaplyMarker.h"
#import "visual_objects/MaplyScreenLabel.h"
#import "visual_objects/MaplyLabel.h"
#import "MaplyVectorObject_private.h"
#import "math/MaplyCoordinate.h"
#import "ImageTexture_private.h"
#import "private/MaplyTapMessage_private.h"
#import <vector>

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

@implementation MaplyInteractionLayer
{
    Maply::MapViewRef mapView;
}

- (instancetype)initWithMapView:(Maply::MapViewRef)inMapView
{
    self = [super initWithView:inMapView];
    if (!self)
        return nil;
    mapView = inMapView;
    
    return self;
}

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene
{
    [super startWithThread:inLayerThread scene:inScene];
    
    layerThread = inLayerThread;
    scene = inScene;
}

- (void)dealloc
{    
}

/// Called by the layer thread to shut a layer down.
/// Clean all your stuff out of the scenegraph and so forth.
- (void)teardown
{
    [super teardown];
}

// Do the logic for a selection
// Runs in the layer thread
- (void)userDidTapLayerThread:(MaplyTapMessage *)msg
{
#if !MAPLY_MINIMAL
    // First, we'll look for labels and markers
    NSMutableArray *retSelectArr = [self selectMultipleLabelsAndMarkersForScreenPoint:msg.touchLoc];
    
    // Next, try the vectors
    NSArray *vecObjs = [self findVectorsInPoint:Point2f(msg.whereGeo.x(),msg.whereGeo.y()) inView:(MaplyBaseViewController*)self.viewController multi:true];
    [retSelectArr addObjectsFromArray:[self convertSelectedVecObjects:vecObjs]];
    
    // Tell the view controller about it
    dispatch_async(dispatch_get_main_queue(),^
                   {
                       [self->_viewController handleSelection:msg didSelect:retSelectArr];
                   }
                   );
#endif //!MAPLY_MINIMAL
}

// Check for a selection
- (void)userDidTap:(MaplyTapMessage *)msg
{
    // Pass it off to the layer thread
    [self performSelector:@selector(userDidTapLayerThread:) onThread:layerThread withObject:msg waitUntilDone:NO];
}

#if !MAPLY_MINIMAL
- (NSMutableArray*)selectMultipleLabelsAndMarkersForScreenPoint:(CGPoint)screenPoint
{
    SelectionManagerRef selectManager = std::dynamic_pointer_cast<SelectionManager>(scene->getManager(kWKSelectionManager));
    std::vector<SelectionManager::SelectedObject> selectedObjs;
    selectManager->pickObjects(Point2f(screenPoint.x,screenPoint.y),10.0,mapView->makeViewState(layerThread.renderer),selectedObjs);

    return [self convertSelectedObjects:selectedObjs];
}
#endif //!MAPLY_MINIMAL

@end
