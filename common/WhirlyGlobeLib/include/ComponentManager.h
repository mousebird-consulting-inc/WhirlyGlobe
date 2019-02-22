/*
 *  ComponentManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/15/19.
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

#import "BillboardManager.h"
#import "GeometryManager.h"
#import "IntersectionManager.h"
#import "LabelManager.h"
#import "LayoutManager.h"
#import "LoftManager.h"
#import "MarkerManager.h"
#import "ParticleSystemManager.h"
#import "SceneGraphManager.h"
#import "ShapeManager.h"
#import "SphericalEarthChunkManager.h"
#import "VectorManager.h"
#import "WideVectorManager.h"
#import "SelectionManager.h"

namespace WhirlyKit
{

/* Component Object is a higher level container for the various
 IDs and objects associated with a single set of geometry (or whatever).
 */
class ComponentObject : public Identifiable
{
public:
    ComponentObject();
    ComponentObject(SimpleIdentity theID);
    virtual ~ComponentObject();
    
    SimpleIDSet markerIDs;
    SimpleIDSet labelIDs;
    SimpleIDSet vectorIDs;
    SimpleIDSet wideVectorIDs;
    SimpleIDSet shapeIDs;
    SimpleIDSet chunkIDs;
    SimpleIDSet loftIDs;
    SimpleIDSet billIDs;
    SimpleIDSet geomIDs;
    SimpleIDSet partSysIDs;
    SimpleIDSet selectIDs;
    SimpleIDSet drawStringIDs;
    // Note: Move vectors in here as well
    Point2d vectorOffset;
    bool isSelectable;
    bool enable;
    bool underConstruction;
    
    // Empty out references
    void clear();
};

typedef std::shared_ptr<ComponentObject> ComponentObjectRef;
    
typedef std::set<ComponentObjectRef,IdentifiableRefSorter> ComponentObjectSet;
    
#define kWKComponentManager "kWKComponentManager"
    
/** Component Object Manager
 
    Manages component objects, particular enable/disable and deletion.
    The
  */
class ComponentManager : public SceneManager
{
public:
    ComponentManager();
    virtual ~ComponentManager();
    
    // Called when the scene sets up the managers
    void setScene(Scene *inScene);
    
    /// Hand a component object over to be managed.
    /// Return an ID to refer to it in the future
    virtual void addComponentObject(ComponentObjectRef compObj);
    
    /// Check if the component object exists
    virtual bool hasComponentObject(SimpleIdentity compID);

    /// Remove the given Component Object and all its associated data
    virtual void removeComponentObject(SimpleIdentity compID,ChangeSet &changes);

    /// Remove a list of Component Objects
    virtual void removeComponentObjects(const SimpleIDSet &compIDs,ChangeSet &changes);

    /// Enable/disable the contents of a Component Object
    virtual void enableComponentObject(SimpleIdentity compID,bool enable,ChangeSet &changes);
    
    /// Enable/disable a whole group of Component Objects
    virtual void enableComponentObjects(const SimpleIDSet &compIDs,bool enable,ChangeSet &changes);
    
    // These are here for convenience
    LayoutManager *layoutManager;
    MarkerManager *markerManager;
    LabelManager *labelManager;
    VectorManager *vectorManager;
    WideVectorManager *wideVectorManager;
    ShapeManager *shapeManager;
    SphericalChunkManager *chunkManager;
    LoftManager *loftManager;
    BillboardManager *billManager;
    GeometryManager *geomManager;
    FontTextureManager *fontTexManager;
    ParticleSystemManager *partSysManager;
    
protected:
    std::mutex lock;

    ComponentObjectSet compObjs;
};

// Make an OS specific component manager
extern ComponentManager *MakeComponentManager();
    
}
