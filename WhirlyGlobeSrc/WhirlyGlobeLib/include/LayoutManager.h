/*
 *  LayoutManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/15/13.
 *  Copyright 2011-2013 mousebird consulting. All rights reserved.
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
#import "Scene.h"
#import "SceneRendererES.h"
#import "ViewState.h"
#import "ScreenSpaceBuilder.h"
#import "SelectionManager.h"

namespace WhirlyKit
{

/// Okay to place to the right of a point
#define WhirlyKitLayoutPlacementRight  (1<<0)
/// Okay to place it to the left of a point
#define WhirlyKitLayoutPlacementLeft   (1<<1)
/// Okay to place on top of a point
#define WhirlyKitLayoutPlacementAbove  (1<<2)
/// Okay to place below a point
#define WhirlyKitLayoutPlacementBelow  (1<<3)

/** This represents an object in the screen space generator to be laid out
 by the layout engine.  We'll manipulate its offset and enable/disable it
 but won't otherwise change it.
 */
class LayoutObject : public ScreenSpaceObject
{
public:
    LayoutObject();
    LayoutObject(SimpleIdentity theId);
    
    // Set the layout size from width/height
    void setLayoutSize(const Point2d &layoutSize,const Point2d &offset);
    
    // Set the selection size from width/height
    void setSelectSize(const Point2d &layoutSize,const Point2d &offset);

    // Size to use for laying out
    std::vector<Point2d> layoutPts;
    
    // Size to use for selection
    std::vector<Point2d> selectPts;

    /// This is used to sort objects for layout.  Bigger is more important.
    float importance;
    /// Options for where to place this object:  WhirlyKitLayoutPlacementLeft, WhirlyKitLayoutPlacementRight,
    ///  WhirlyKitLayoutPlacementAbove, WhirlyKitLayoutPlacementBelow
    int acceptablePlacement;
};

// Private fields we use for object layout
class LayoutObjectEntry : public Identifiable
{
public:
    LayoutObjectEntry(SimpleIdentity theId)
    : Identifiable(theId)
    {
        currentEnable = newEnable = false;
        offset = Point2d(MAXFLOAT,MAXFLOAT);
        changed = true;
    }
    
    // The layout objects as passed in by the original caller
    LayoutObject obj;
    
    // Set if it's currently on
    bool currentEnable;
    // Set if it's going to be on
    bool newEnable;
    // The offset, as calculated
    WhirlyKit::Point2d offset;
    // Set if we changed something during evaluation
    bool changed;
};

typedef std::set<LayoutObjectEntry *,IdentifiableSorter> LayoutEntrySet;
    
#define kWKLayoutManager "WKLayoutManager"

/** The layout manager handles 2D text and marker layout.  We feed it objects
    we want to be drawn and it will figure out which ones should be visible
    and which shouldn't.
 
    This manager is entirely thread safe except for destruction.
  */
class LayoutManager: public SceneManager
{
public:
    LayoutManager();
    virtual ~LayoutManager();
    
    /// If set, the maximum number of objects to display
    void setMaxDisplayObjects(int numObjects);
    
    /// Add objects for layout (thread safe)
    void addLayoutObjects(const std::vector<LayoutObject> &newObjects);
    
        /// Add objects for layout (thread safe)
    void addLayoutObjects(const std::vector<LayoutObject *> &newObjects);
    
    /// Remove objects for layout (thread safe)
    void removeLayoutObjects(const SimpleIDSet &oldObjects);
    
    /// Enable/disable layout objects
    void enableLayoutObjects(const SimpleIDSet &layoutObjects,bool enable);
    
    /// Run the layout logic for everything we're aware of (thread safe)
    void updateLayout(WhirlyKit::ViewState *viewState,ChangeSet &changes);
    
    /// True if we've got changes since the last update
    bool hasChanges();
    
        /// Return the active objects in a form the selection manager can handle
    void getScreenSpaceObjects(const SelectionManager::PlacementInfo &pInfo,std::vector<ScreenSpaceObjectLocation> &screenSpaceObjs);
        
protected:
    void runLayoutRules(WhirlyKit::ViewState *viewState);
    
    pthread_mutex_t layoutLock;
    /// If non-zero the maximum number of objects we'll display at once
    int maxDisplayObjects;
    /// If there were updates since the last layout
    bool hasUpdates;
    /// Objects we're controlling the placement for
    LayoutEntrySet layoutObjects;
    /// Drawables created on the last round
    SimpleIDSet drawIDs;
};

}
