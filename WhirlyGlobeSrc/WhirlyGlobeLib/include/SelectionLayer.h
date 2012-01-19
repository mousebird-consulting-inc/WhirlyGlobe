/*
 *  SelectionLayer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/26/11.
 *  Copyright 2011 mousebird consulting. All rights reserved.
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
#import "SceneRendererES1.h"

namespace WhirlyGlobe
{

/** Rectangle Selectable.
    This is used internal to the selection layer to track a
    selectable rectangle.  It consists of geometry and an
    ID to track it.
  */
class RectSelectable
{
public:
    
    // Comparison operator for sorting
    bool operator < (const RectSelectable &that) const;
    
    // Used to identify this selectable
    SimpleIdentity selectID;
    Point3f pts[4];  // Geometry
    Vector3f norm;   // Calculate normal
    float minVis,maxVis;  // Range over which this is visible
};

typedef std::set<WhirlyGlobe::RectSelectable> RectSelectableSet;
 
}

/** The selection layer tracks a variable number of objects that
    might be selectable.  These consist of a shape and an ID.
    Other layers (or the caller) can register objects with the
    selection layer.  These objects will be considered for selection
    when the caller uses pickObject.
 
    All objects are currently being projected to the 2D screen and
    evaluated for distance there.
 */
@interface WhirlyGlobeSelectionLayer : NSObject<WhirlyGlobeLayer>
{
    /// The view controls how the globe/map is displayed
    WhirlyKitView *theView;
    /// The renderer has screen size information
    WhirlyGlobeSceneRendererES1 *renderer;
    /// Layer thread we're associated with
    WhirlyGlobeLayerThread *layerThread;
    /// The selectable objects themselves
    WhirlyGlobe::RectSelectableSet selectables;
}

/// Construct with a globe view.  Need that for screen space calculations
- (id)initWithView:(WhirlyKitView *)inView renderer:(WhirlyGlobeSceneRendererES1 *)inRenderer;

/// Called in the layer thread
- (void)startWithThread:(WhirlyGlobeLayerThread *)layerThread scene:(WhirlyGlobe::GlobeScene *)scene;

/// Add a rectangle (in 3-space) always available for selection
- (void)addSelectableRect:(WhirlyGlobe::SimpleIdentity)selectId rect:(WhirlyGlobe::Point3f *)pts;

/// Add a rectangle (in 3-space) for selection, but only between the given visibilities
- (void)addSelectableRect:(WhirlyGlobe::SimpleIdentity)selectId rect:(WhirlyGlobe::Point3f *)pts minVis:(float)minVis maxVis:(float)maxVis;

/// Remove the given selectable from consideration
- (void)removeSelectable:(WhirlyGlobe::SimpleIdentity)selectId;

/// Pass in the screen point where the user touched.  This returns the closest hit within the given distance
- (WhirlyGlobe::SimpleIdentity)pickObject:(WhirlyGlobe::Point2f)touchPt maxDist:(float)maxDist;

@end
