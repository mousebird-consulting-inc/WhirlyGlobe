/*
 *  SceneGraphManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/24/12.
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

#import <math.h>
#import <set>
#import <boost/shared_ptr.hpp>
#import <boost/pointer_cast.hpp>
#import "WhirlyVector.h"
#import "Identifiable.h"
#import "Drawable.h"
#import "Generator.h"
#import "LayerViewWatcher.h"
#import "DynamicDrawableAtlas.h"
#import "Scene.h"

namespace WhirlyKit
{
    
class SceneGraphGroup;
    
// Base class for scenegraph nodes
class SceneGraphNode
{
public:
    SceneGraphNode() : parent(NULL) { }
    virtual ~SceneGraphNode() { }
    SceneGraphGroup *parent;
};
    
// Container for drawables
class SceneGraphGeometry : public SceneGraphNode
{
public:
    SceneGraphGeometry() : isDisplayed(false) { }
    virtual ~SceneGraphGeometry() {  }
    
    void addDrawable(SimpleIdentity drawID) { drawIDs.insert(drawID); }
    
    // Set if we're already displaying this
    bool isDisplayed;
    SimpleIDSet drawIDs;
};

// Simple group that has a number of children
class SceneGraphGroup : public SceneGraphNode, public WhirlyKit::Identifiable
{
public:
    SceneGraphGroup() : numExpectedChildren(0) { }
    SceneGraphGroup(SimpleIdentity theId) : Identifiable(theId) { }
    virtual ~SceneGraphGroup()
    {
        // No longer need this.  We're always deleting all the nodes in the manager
//        for (std::set<SceneGraphNode *>::iterator it = nodes.begin();
//             it != nodes.end(); ++it)
//            delete *it;
        nodes.clear();
    }
    
    void addChild(SceneGraphNode *child) { nodes.insert(child); child->parent = this; }
    void removeChild(SceneGraphNode *child) { child->parent = NULL; nodes.erase(child); }
    
    int numExpectedChildren;
    std::set<SceneGraphNode *> nodes;
};    

// Level of detail node
class SceneGraphLOD : public SceneGraphGroup
{
public:
    SceneGraphLOD() { numExpectedChildren = 0; }
    
    float switchIn,switchOut;
    WhirlyKit::Point3f center;
};

// Manages a scene graph
class SceneGraphManager : public WhirlyKit::Generator
{
public:
    SceneGraphManager(DynamicDrawableAtlas *drawAtlas);
    virtual ~SceneGraphManager();
    
    /// Add a drawable to be referenced by the scenegraph
    void addDrawable(BasicDrawable *draw,ChangeSet &changes);
    
    /// Add the given scenegraph fragment
    void attachSceneFragment(SimpleIdentity attachID,SceneGraphNode *node);

    /// Remove the given scenegraph fragment (by ID)
    void removeSceneFragment(SimpleIdentity nodeID,ChangeSet &changes);
    
    // Remove the given drawable by ID (if it's there)
    void removeDrawable(SimpleIdentity drawID,ChangeSet &changes);
    
    /// Run the position calculations and update what we'll display.
    /// The changes need to be flushed by the caller
    void update(WhirlyKitViewState *viewState,ChangeSet &changes);
    
    /// Print out stats for debugging
    void dumpStats();
    
protected:
    // Recursive traversal for a scenegraph node
    void traverseNode(WhirlyKit::Point3f localPt,SceneGraphNode *node,std::set<SceneGraphNode *> &siblingNodes,SimpleIDSet &toDraw);
    
    // Recursively add Node IDs to our full set of node IDs
    void traverseAddNodeIDs(SceneGraphNode *node);
    
    // Remove the various Node IDS from our full set
    void traverseRemNodeIDs(SceneGraphNode *node,ChangeSet &changes);
    
    // Look for a drawable by ID
    Drawable *getDrawable(SimpleIdentity drawID);
    
    // Top level nodes in the scenegraph
    typedef std::set<SceneGraphGroup *,WhirlyKit::IdentifiableSorter> NodeSet;
    NodeSet topNodes;
    
    // All group nodes indexed by ID
    NodeSet allNodes;
    
    // All the drawables in the scenegraph (only used if we're not in atlas mode)
    std::set<BasicDrawable *,IdentifiableSorter> drawables;
    
    // Drawables that are currently being drawn
    SimpleIDSet activeDrawIDs;
    
    // Drawable atlas we used to draw this stuff
    DynamicDrawableAtlas *drawAtlas;
};

}
