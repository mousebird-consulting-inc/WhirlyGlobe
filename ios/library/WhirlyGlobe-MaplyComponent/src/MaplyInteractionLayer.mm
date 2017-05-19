/*
 *  MaplyInteractionLayer_private.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/21/12.
 *  Copyright 2011-2017 mousebird consulting
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

#import "MaplyInteractionLayer_private.h"
#import "MaplyScreenMarker.h"
#import "MaplyMarker.h"
#import "MaplyScreenLabel.h"
#import "MaplyLabel.h"
#import "MaplyVectorObject_private.h"
#import "MaplyCoordinate.h"
#import "ImageTexture_private.h"
#import <vector>

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

@implementation MaplyInteractionLayer
{
    Maply::MapScene *mapScene;
    MaplyView *mapView;
}

- (instancetype)initWithMapView:(MaplyView *)inMapView
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
    mapScene = (Maply::MapScene *)inScene;
    
    layerThread = inLayerThread;
    scene = (Maply::MapScene *)inScene;
    userObjects = [NSMutableSet set];
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
- (void) userDidTapLayerThread:(MaplyTapMessage *)msg
{
    // First, we'll look for labels and markers
    NSMutableArray *retSelectArr = [self selectMultipleLabelsAndMarkersForScreenPoint:msg.touchLoc];
    
    // Next, try the vectors
    NSArray *vecObjs = [self findVectorsInPoint:Point2f(msg.whereGeo.x(),msg.whereGeo.y()) inView:(MaplyBaseViewController*)self.viewController multi:true];
    for (MaplyVectorObject *vecObj in vecObjs)
    {
        MaplySelectedObject *selObj = [[MaplySelectedObject alloc] init];
        selObj.selectedObj = vecObj;
        selObj.screenDist = 0.0;
        // Note: Not quite right
        selObj.zDist = 0.0;
        [retSelectArr addObject:selObj];
    }
    
    // Tell the view controller about it
    dispatch_async(dispatch_get_main_queue(),^
                   {
                       [_viewController handleSelection:msg didSelect:retSelectArr];
                   }
                   );
}

- (NSMutableArray*)selectMultipleLabelsAndMarkersForScreenPoint:(CGPoint)screenPoint
{
    SelectionManager *selectManager = (SelectionManager *)scene->getManager(kWKSelectionManager);
    std::vector<SelectionManager::SelectedObject> selectedObjs;
    selectManager->pickObjects(Point2f(screenPoint.x,screenPoint.y),10.0,mapView,selectedObjs);
    
    NSMutableArray *retSelectArr = [NSMutableArray array];
    if (!selectedObjs.empty())
    {
        // Work through the objects the manager found, creating entries for each
        for (unsigned int ii=0;ii<selectedObjs.size();ii++)
        {
            SelectionManager::SelectedObject &theSelObj = selectedObjs[ii];
            MaplySelectedObject *selObj = [[MaplySelectedObject alloc] init];
            
            for (auto selectID : theSelObj.selectIDs)
            {
                SelectObjectSet::iterator it = selectObjectSet.find(SelectObject(selectID));
                if (it != selectObjectSet.end())
                    selObj.selectedObj = it->obj;
                
                selObj.screenDist = theSelObj.screenDist;
                selObj.cluster = theSelObj.isCluster;
                selObj.zDist = theSelObj.distIn3D;
                
                if (selObj.selectedObj)
                    [retSelectArr addObject:selObj];
            }
        }
    }
    
    return retSelectArr;
}

// Check for a selection
- (void) userDidTap:(MaplyTapMessage *)msg
{
    // Pass it off to the layer thread
    [self performSelector:@selector(userDidTapLayerThread:) onThread:layerThread withObject:msg waitUntilDone:NO];
}

@end
