/*
 *  SceneGraphGenerator.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/24/12.
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

#import <math.h>
#import <set>
#import <boost/shared_ptr.hpp>
#import <boost/pointer_cast.hpp>
#import "WhirlyVector.h"
#import "Identifiable.h"
#import "Drawable.h"
#import "Generator.h"

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
    
typedef boost::shared_ptr<SceneGraphNode> SceneGraphNodeRef;
typedef boost::shared_ptr<WhirlyKit::Drawable> DrawableRef;

// Container for drawables
class SceneGraphGeometry : public SceneGraphNode
{
public:
    SceneGraphGeometry() { }
    virtual ~SceneGraphGeometry() { drawables.clear(); }
    
    void addDrawable(WhirlyKit::DrawableRef draw) { drawables.push_back(draw); }
    
    std::vector<WhirlyKit::DrawableRef> drawables;
};
typedef boost::shared_ptr<SceneGraphGeometry> SceneGraphGeometryRef;

// Simple group that has a number of children
class SceneGraphGroup : public SceneGraphNode, public WhirlyKit::Identifiable
{
public:
    SceneGraphGroup() : numExpectedChildren(0) { }
    SceneGraphGroup(SimpleIdentity theId) : Identifiable(theId) { }
    virtual ~SceneGraphGroup() { nodes.clear(); }
    
    void addChild(SceneGraphNodeRef child) { nodes.insert(child); child->parent = this; }
    void removeChild(SceneGraphNodeRef child) { child->parent = NULL; nodes.erase(child); }
    
    int numExpectedChildren;
    std::set<SceneGraphNodeRef> nodes;
};    
typedef boost::shared_ptr<SceneGraphGroup> SceneGraphGroupRef;

// Level of detail node
class SceneGraphLOD : public SceneGraphGroup
{
public:
    SceneGraphLOD() { numExpectedChildren = 0; }
    
    float switchIn,switchOut;
    WhirlyKit::Point3f center;
};
typedef boost::shared_ptr<SceneGraphLOD> SceneGraphLODRef;

// Manages a scene graph
class SceneGraphGenerator : public WhirlyKit::Generator
{
    friend class SceneGraphGeneratorAddRequest;
    friend class SceneGraphGeneratorRemRequest;
public:
    SceneGraphGenerator();
    virtual ~SceneGraphGenerator();

    /// Generate the drawables for the given frame
    void generateDrawables(WhirlyKitRendererFrameInfo *frameInfo,std::vector<WhirlyKit::DrawableRef> &drawables,std::vector<WhirlyKit::DrawableRef> &screenDrawables); 
    
    /// Print out stats for debugging
    void dumpStats();
    
protected:
    // Recursive traversal for a scenegraph node
    void traverseNode(WhirlyKitRendererFrameInfo *frameInfo,WhirlyKit::Point3f localPt,SceneGraphNodeRef node,std::set<SceneGraphNodeRef> &siblingNodes,std::vector<WhirlyKit::DrawableRef> &drawables);
    
    // Recursively add Node IDs to our full set of node IDs
    void traverseAddNodeIDs(SceneGraphNodeRef node);
    
    // Remove the various Node IDS from our full set
    void traverseRemNodeIDs(SceneGraphNodeRef node);
    
    // Add the given scenegraph fragment
    void attachSceneFragment(SimpleIdentity attachID,SceneGraphNodeRef node);
    
    // Remove the given scenegraph fragment (by ID)
    void removeSceneFragment(SimpleIdentity nodeID);
    
    // Top level nodes in the scenegraph
    typedef std::set<SceneGraphGroupRef,WhirlyKit::IdentifiableRefSorter> NodeRefSet;
    NodeRefSet topNodes;
    
    // All group nodes indexed by ID
    typedef std::set<SceneGraphGroupRef,WhirlyKit::IdentifiableRefSorter> NodeSet;
    NodeSet allNodes;
};

// Attach a given scenegraph to a group or the top
class SceneGraphGeneratorAddRequest : public WhirlyKit::GeneratorChangeRequest
{
public:
    SceneGraphGeneratorAddRequest(SimpleIdentity genID,SimpleIdentity attachID,SceneGraphNodeRef node);
    
    virtual void execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,Generator *gen);
protected:
    SimpleIdentity attachID;
    SceneGraphNodeRef node;
};
    
// Remove the given scenegraph from wherever it might be
class SceneGraphGeneratorRemRequest : public GeneratorChangeRequest
{
public:
    SceneGraphGeneratorRemRequest(SimpleIdentity genID,SimpleIdentity nodeID);
    
    virtual void execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,Generator *gen);
protected:
    SimpleIdentity nodeID;
};

}
