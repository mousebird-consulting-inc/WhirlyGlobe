/*
 *  MaplyInteractionLayer_private.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/21/12.
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

#import "MaplyInteractionLayer_private.h"
// Note: Porting
//#import "MaplyScreenMarker.h"
//#import "MaplyMarker.h"
//#import "MaplyScreenLabel.h"
//#import "MaplyLabel.h"
#import "MaplyVectorObject_private.h"
#import "MaplyCoordinate.h"
// Note: Porting
//#import "ImageTexture_private.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

@implementation MaplyInteractionLayer
{
    Maply::MapScene *mapScene;
    Maply::MapView *mapView;
}

- (id)initWithMapView:(Maply::MapView *)inMapView
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
    userObjects = [NSMutableArray array];
}

- (void)dealloc
{    
}

/// Called by the layer thread to shut a layer down.
/// Clean all your stuff out of the scenegraph and so forth.
- (void)shutdown
{
    [super shutdown];
}

// Note: Porting
//// Do the logic for a selection
//// Runs in the layer thread
//- (void) userDidTapLayerThread:(MaplyTapMessage *)msg
//{
//    // First, we'll look for labels and markers
//    SimpleIdentity selID = ((SelectionManager *)scene->getManager(kWKSelectionManager))->pickObject(Point2f(msg.touchLoc.x,msg.touchLoc.y),10.0,mapView);
//
//    NSObject *selObj;
//    if (selID != EmptyIdentity)
//    {       
//        // Found something.  Now find the associated object
//        SelectObjectSet::iterator it = selectObjectSet.find(SelectObject(selID));
//        if (it != selectObjectSet.end())
//        {
//            selObj = it->obj;
//        }
//    } else {
//        // Next, try the vectors
//        selObj = [self findVectorInPoint:Point2f(msg.whereGeo.x(),msg.whereGeo.y())];
//    }
//    
//    // Tell the view controller about it
//    dispatch_async(dispatch_get_main_queue(),^
//                   {
//                       [_viewController handleSelection:msg didSelect:selObj];
//                   }
//                   );
//}
//
//// Check for a selection
//- (void) userDidTap:(MaplyTapMessage *)msg
//{
//    // Pass it off to the layer thread
//    [self performSelector:@selector(userDidTapLayerThread:) onThread:layerThread withObject:msg waitUntilDone:NO];
//}

@end