/*
 *  WGInteractionLayer_private.h
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

#import "WGInteractionLayer_private.h"
#import "MaplyScreenMarker.h"
#import "MaplyMarker.h"
#import "MaplyScreenLabel.h"
#import "MaplyLabel.h"
#import "MaplyVectorObject_private.h"
#import "MaplyShape.h"
#import "MaplySticker.h"
#import "WGCoordinate.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

@implementation WGInteractionLayer
{
    AnimateViewMomentum *autoSpinner;
}

// Initialize with the globeView
-(id)initWithGlobeView:(WhirlyGlobeView *)inGlobeView
{
    self = [super initWithView:inGlobeView];
    if (!self)
        return nil;
    globeView = inGlobeView;
    
    return self;
}

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene
{
    [super startWithThread:inLayerThread scene:inScene];
        
    // Run the auto spin every so often
    [self performSelector:@selector(processAutoSpin:) withObject:nil afterDelay:1.0];
}

/// Called by the layer thread to shut a layer down.
/// Clean all your stuff out of the scenegraph and so forth.
- (void)shutdown
{
    [super shutdown];
}

- (void)setAutoRotateInterval:(float)inAutoRotateInterval degrees:(float)inAutoRotateDegrees
{
    autoRotateInterval = inAutoRotateInterval;
    autoRotateDegrees = inAutoRotateDegrees;
    if (autoSpinner)
    {
        if (autoRotateInterval == 0.0 || autoRotateDegrees == 0)
        {
            [globeView cancelAnimation];
            autoSpinner = nil;
        } else
            // Update the spin
            autoSpinner.velocity = autoRotateDegrees / 180.0 * M_PI;
    }
}

// Try to auto-spin every so often
-(void)processAutoSpin:(id)sender
{
    NSTimeInterval now = CFAbsoluteTimeGetCurrent();
    
    if (autoSpinner && globeView.delegate != autoSpinner)
        autoSpinner = nil;
    
    if (autoRotateInterval > 0.0 && !autoSpinner)
    {
        if (now - globeView.lastChangedTime > autoRotateInterval &&
            now - lastTouched > autoRotateInterval)
        {
            float anglePerSec = autoRotateDegrees / 180.0 * M_PI;
            
            // Keep going in that direction
            Vector3f upVector(0,0,1);
            autoSpinner = [[AnimateViewMomentum alloc] initWithView:globeView velocity:anglePerSec accel:0.0 axis:upVector northUp:false];
            globeView.delegate = autoSpinner;
        }
    }
    
    [self performSelector:@selector(processAutoSpin:) withObject:nil afterDelay:1.0];
}

// Do the logic for a selection
// Runs in the layer thread
- (void) userDidTapLayerThread:(WhirlyGlobeTapMessage *)msg
{
    lastTouched = CFAbsoluteTimeGetCurrent();
    if (autoSpinner)
    {
        if (globeView.delegate == autoSpinner)
        {
            autoSpinner = nil;
            globeView.delegate = nil;
        }
    }
    
    // First, we'll look for labels and markers
    SelectionManager *selectManager = (SelectionManager *)scene->getManager(kWKSelectionManager);
    std::vector<SelectionManager::SelectedObject> selectedObjs;
    selectManager->pickObjects(Point2f(msg.touchLoc.x,msg.touchLoc.y),10.0,globeView,selectedObjs);

    NSMutableArray *retSelectArr = [NSMutableArray array];
    if (!selectedObjs.empty())
    {
        // Work through the objects the manager found, creating entries for each
        for (unsigned int ii=0;ii<selectedObjs.size();ii++)
        {
            SelectionManager::SelectedObject &theSelObj = selectedObjs[ii];
            MaplySelectedObject *selObj = [[MaplySelectedObject alloc] init];

            SelectObjectSet::iterator it = selectObjectSet.find(SelectObject(theSelObj.selectID));
            if (it != selectObjectSet.end())
                selObj.selectedObj = it->obj;

            selObj.screenDist = theSelObj.screenDist;
            selObj.zDist = theSelObj.distIn3D;
            
            if (selObj.selectedObj)
                [retSelectArr addObject:selObj];
        }
        
        // Found something.  Now find the associated object
    }
    
    // Next, try the vectors
    // Note: This means we'll never get both vectors and other objects
    NSArray *vecObjs = [self findVectorsInPoint:Point2f(msg.whereGeo.x(),msg.whereGeo.y())];
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

// Check for a selection
- (void) userDidTap:(WhirlyGlobeTapMessage *)msg
{
    // Pass it off to the layer thread
    [self performSelector:@selector(userDidTapLayerThread:) onThread:layerThread withObject:msg waitUntilDone:NO];
}

@end