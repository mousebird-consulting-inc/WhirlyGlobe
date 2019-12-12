/*
 *  SceneGraphManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/24/12.
 *  Copyright 2011-2019 mousebird consulting
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
#import "WhirlyVector.h"
#import "Identifiable.h"
#import "BasicDrawable.h"
#import "Scene.h"

namespace WhirlyKit
{
    
class SceneGraphGroup;
class SceneGraphManager;
typedef std::set<SceneGraphGroup *,WhirlyKit::IdentifiableSorter> SceneGraphNodeSet;

// Base class for scenegraph nodes
class SceneGraphNode
{
public:
    SceneGraphNode() : parent(NULL) { }
    virtual ~SceneGraphNode() { }
    SceneGraphGroup *parent;
    
    // Used to build up the geometry we're to draw
    virtual void traverseNodeDrawables(SceneGraphManager *manage,const Point3f &localPt,const std::set<SceneGraphNode *> &siblingNodes,SimpleIDSet &toDraw) { }
    
    // Returns all the node IDs
    virtual void traverseAddNodeIDs(SceneGraphManager *manage,SceneGraphNodeSet &allNodes) { }
    
    // Removes all the node IDs and drawables
    virtual void traverseRemNodeIDs(SceneGraphManager *manage,SceneGraphNodeSet &allNodes,ChangeSet &changes) { }
};
    
// Container for drawables
class SceneGraphGeometry : public SceneGraphNode
{
public:
    SceneGraphGeometry() : isDisplayed(false) { }
    virtual ~SceneGraphGeometry() {  }
    
    void addDrawable(SimpleIdentity drawID) { drawIDs.insert(drawID); }
        
    void traverseNodeDrawables(SceneGraphManager *manage,const Point3f &localPt,const std::set<SceneGraphNode *> &siblingNodes,SimpleIDSet &toDraw);
    void traverseRemNodeIDs(SceneGraphManager *manage,SceneGraphNodeSet &allNodes,ChangeSet &changes);

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
        for (std::set<SceneGraphNode *>::iterator it = nodes.begin();
             it != nodes.end(); ++it)
            delete *it;
        nodes.clear();
    }
    
    void addChild(SceneGraphNode *child) { nodes.insert(child); child->parent = this; }
    void removeChild(SceneGraphNode *child) { child->parent = NULL; nodes.erase(child); }

    void traverseNodeDrawables(SceneGraphManager *manage,const Point3f &localPt,const std::set<SceneGraphNode *> &siblingNodes,SimpleIDSet &toDraw);
    void traverseAddNodeIDs(SceneGraphManager *manage,SceneGraphNodeSet &allNodes);
    void traverseRemNodeIDs(SceneGraphManager *manage,SceneGraphNodeSet &allNodes,ChangeSet &changes);

    int numExpectedChildren;
    std::set<SceneGraphNode *> nodes;
};    

// Level of detail node
class SceneGraphLOD : public SceneGraphGroup
{
public:
    SceneGraphLOD() { numExpectedChildren = 0; }
    
    void traverseNodeDrawables(SceneGraphManager *manage,const Point3f &localPt,const std::set<SceneGraphNode *> &siblingNodes,SimpleIDSet &toDraw);
    
    float switchIn,switchOut;
    WhirlyKit::Point3f center;
};

// Manages a scene graph
class SceneGraphManager : DelayedDeletable
{
public:
    SceneGraphManager();
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
    void update(ViewStateRef viewState,ChangeSet &changes);
    
    /// Print out stats for debugging
    void dumpStats();

protected:
    // Top level nodes in the scenegraph
    SceneGraphNodeSet topNodes;
    
    // All group nodes indexed by ID
    SceneGraphNodeSet allNodes;
    
    // All the drawables in the scenegraph (only used if we're not in atlas mode)
    std::set<SimpleIdentity> drawables;
    
    // Drawables that are currently being drawn
    SimpleIDSet activeDrawIDs;    
};

}
