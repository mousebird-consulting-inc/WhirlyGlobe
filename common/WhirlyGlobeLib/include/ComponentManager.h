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
#import "VectorObject.h"
#import "WideVectorManager.h"
#import "SelectionManager.h"

namespace WhirlyKit
{

/* Component Object is a higher level container for the various
 IDs and objects associated with a single set of geometry (or whatever).
 */
class ComponentObject : public Identifiable
{
    friend class ComponentManager;
public:
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
    
    // Vectors objects associated with this component object
    std::vector<VectorObjectRef> vecObjs;
    
    Point2d vectorOffset;
    bool isSelectable;
    bool enable;
    bool underConstruction;
    
    // Empty out references
    void clear();
    
public:
    // Don't call this
    ComponentObject();
};

typedef std::shared_ptr<ComponentObject> ComponentObjectRef;
    
typedef std::map<SimpleIdentity,ComponentObjectRef> ComponentObjectMap;
    
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
    virtual void removeComponentObject(PlatformThreadInfo *threadInfo,SimpleIdentity compID,ChangeSet &changes);

    /// Remove a list of Component Objects
    virtual void removeComponentObjects(PlatformThreadInfo *threadInfo,const SimpleIDSet &compIDs,ChangeSet &changes);
    
    /// Remove a vector of Component Objects
    virtual void removeComponentObjects(PlatformThreadInfo *threadInfo,const std::vector<ComponentObjectRef> &compObjs,ChangeSet &changes);

    /// Enable/disable the contents of a Component Object
    virtual void enableComponentObject(SimpleIdentity compID,bool enable,ChangeSet &changes);
    
    /// Enable/disable a whole group of Component Objects
    virtual void enableComponentObjects(const SimpleIDSet &compIDs,bool enable,ChangeSet &changes);
    
    /// Set a uniform block on the geometry for the given component objects
    virtual void setUniformBlock(const SimpleIDSet &compIDs,const RawDataRef &uniBlock,int bufferID,ChangeSet &changes);
    
    /// Find all the vectors that fall within or near the given point
    std::vector<std::pair<ComponentObjectRef,VectorObjectRef> > findVectors(const Point2d &pt,double maxDist,ViewStateRef viewState,const Point2f &frameSize,bool muti);
    
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
    // Subclass fills this in
    virtual ComponentObjectRef makeComponentObject() = 0;
    
    std::mutex lock;

    ComponentObjectMap compObjs;
};

// Make an OS specific component manager
extern ComponentManager *MakeComponentManager();
    
}
