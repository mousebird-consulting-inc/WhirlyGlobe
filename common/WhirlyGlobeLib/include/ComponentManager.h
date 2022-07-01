/*
 *  ComponentManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/15/19.
 *  Copyright 2011-2022 mousebird consulting
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

#import "ShapeManager.h"

#if !MAPLY_MINIMAL
# import "BillboardManager.h"
# import "GeometryManager.h"
# import "IntersectionManager.h"
# import "LabelManager.h"
# import "LayoutManager.h"
# import "LoftManager.h"
# import "MarkerManager.h"
# import "ParticleSystemManager.h"
# import "SceneGraphManager.h"
# import "SphericalEarthChunkManager.h"
# import "VectorManager.h"
# import "VectorObject.h"
# import "WideVectorManager.h"
# import "SelectionManager.h"
#endif //!MAPLY_MINIMAL

namespace WhirlyKit
{

/* Component Object is a higher level container for the various
 IDs and objects associated with a single set of geometry (or whatever).
 */
class ComponentObject : public Identifiable
{
    friend class ComponentManager;
public:
    virtual ~ComponentObject() = default;
    
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
    
    Point2d vectorOffset = { 0, 0, };
    
    std::string uuid;
    std::string representation;
    
    // If the object uses masks, these are the masks in use
    SimpleIDSet maskIDs;

    bool enable;
    bool isSelectable;
    bool underConstruction = false;
    
    // Empty out references
    void clear();
    
public:
    // Don't call this
    ComponentObject(bool enable = false, bool selectable = false);
    ComponentObject(bool enable, bool selectable, const Dictionary &desc);
};

typedef std::shared_ptr<ComponentObject> ComponentObjectRef;
using ComponentObjectRefVec = std::vector<ComponentObjectRef>;
    
typedef std::map<SimpleIdentity,ComponentObjectRef> ComponentObjectMap;
    
#define kWKComponentManager "kWKComponentManager"
    
/** Component Object Manager
 
    Manages component objects, particular enable/disable and deletion.
  */
class ComponentManager : public SceneManager
{
public:
    ComponentManager() = default;
    virtual ~ComponentManager() = default;
    
    // Called when the scene sets up the managers
    void setScene(Scene *inScene);
    
    /// Hand a component object over to be managed.
    /// Return an ID to refer to it in the future
    virtual void addComponentObject(const ComponentObjectRef &compObj, ChangeSet &changes);
    
    /// Check if the component object exists
    virtual bool hasComponentObject(SimpleIdentity compID);

    /// Remove the given Component Object and all its associated data
    virtual void removeComponentObject(PlatformThreadInfo *threadInfo,
                                       SimpleIdentity compID,
                                       ChangeSet &changes,
                                       bool disposeAfterRemoval = true);

    /// Remove a list of Component Objects
    virtual void removeComponentObjects(PlatformThreadInfo *threadInfo,
                                        const SimpleIDSet &compIDs,
                                        ChangeSet &changes,
                                        bool disposeAfterRemoval = true);
    
    /// Remove a vector of Component Objects
    virtual void removeComponentObjects(PlatformThreadInfo *threadInfo,
                                        const std::vector<ComponentObjectRef> &compObjs,
                                        ChangeSet &changes,
                                        bool disposeAfterRemoval = true);

    /// Enable/disable the contents of a Component Object
    virtual void enableComponentObject(SimpleIdentity compID,bool enable,ChangeSet &changes, bool resolveReps = false);

    /// Enable/disable the contents of a Component Object
    virtual void enableComponentObject(const ComponentObjectRef &compID, bool enable, ChangeSet &changes, bool resolveReps = false);

    /// Enable/disable the contents of a collection of Component Objects
    virtual void enableComponentObjects(const std::vector<ComponentObjectRef> &compIDs, bool enable, ChangeSet &changes, bool resolveReps = false);

    /// Enable/disable a whole group of Component Objects
    virtual void enableComponentObjects(const SimpleIDSet &compIDs,bool enable,ChangeSet &changes, bool resolveReps = false);

    virtual void setRepresentation(const std::string &repName, const std::string &fallback,
                                   const std::vector<std::string> &uuids, ChangeSet &changes);

    virtual void setRepresentation(const std::string &repName, const std::string &fallback,
                                   const std::set<std::string> &uuids, ChangeSet &changes);

    virtual void setRepresentation(const std::string &repName, const std::string &fallback,
                                   const std::unordered_set<std::string> &uuids, ChangeSet &changes);

    /// Set a uniform block on the geometry for the given component objects
    virtual void setUniformBlock(const SimpleIDSet &compIDs,const RawDataRef &uniBlock,int bufferID,ChangeSet &changes);
    
    /// Pass in a mask name, get an ID to render into the mask target
    virtual SimpleIdentity retainMaskByName(const std::string &maskName);

    /// We're done with the given mask target
    virtual void releaseMaskIDs(const SimpleIDSet &maskIDs);

    using CompObjVectorObjPair = std::pair<ComponentObjectRef,VectorObjectRef>;

    /** Find all the vectors that fall within or near the given point
     *
     *  @param pt The point to search in geographic coordinates
     *  @param maxDist The maximum distance from the point within which a vector must be. (todo: units?)
     *  @param viewState The view state to use for scaling
     *  @param frameSize The frame size in device coordinates to use for scaling
     *  @param multi If true, return all matches; if false return the first match.
     */
    std::vector<CompObjVectorObjPair> findVectors(
            const Point2d &pt,double maxDist,const ViewStateRef &viewState,
            const Point2f &frameSize,bool multi) {
        return findVectors(pt,maxDist,viewState,frameSize,multi?0:1);
    }

    /** Find all the vectors that fall within or near the given point
     *
     *  @param pt The point to search in geographic coordinates
     *  @param maxDist The maximum distance from the point, in device units.
     *  @param viewState The view state to use for scaling
     *  @param frameSize The frame size in device coordinates to use for scaling
     *  @param resultLimit The maximum number of results to return, zero or less for unlimited.
     */
    std::vector<CompObjVectorObjPair> findVectors(
            const Point2d &pt,double maxDist,const ViewStateRef &viewState,
            const Point2f &frameSize,int resultLimit = 0);

    // These are here for convenience
    ShapeManagerRef shapeManager;
#if !MAPLY_MINIMAL
    LayoutManagerRef layoutManager;
    MarkerManagerRef markerManager;
    LabelManagerRef labelManager;
    VectorManagerRef vectorManager;
    WideVectorManagerRef wideVectorManager;
    SphericalChunkManagerRef chunkManager;
    LoftManagerRef loftManager;
    BillboardManagerRef billManager;
    GeometryManagerRef geomManager;
    ParticleSystemManagerRef partSysManager;
#endif //!MAPLY_MINIMAL

protected:
    // Subclass fills this in
    virtual ComponentObjectRef makeComponentObject(const Dictionary *desc = nullptr) = 0;

    void removeComponentObjects_NoLock(PlatformThreadInfo *threadInfo,
                                       const SimpleIDSet &compIDs,
                                       std::vector<ComponentObjectRef> &objs);

    template <typename TIter>
    void setRepresentation(const std::string &repName,
                           const std::string &fallback,
                           TIter beg, TIter end,
                           ChangeSet &changes);

    ComponentObjectMap compObjsById;

    std::unordered_multimap<std::string, ComponentObjectRef> compObjsByUUID;

    std::unordered_map<std::string, std::string> representations;
    
    // Single entry for a mask ID
    class MaskEntry {
    public:
        std::string name;
        SimpleIdentity maskID;
        unsigned long long refCount;
    };
    typedef std::shared_ptr<MaskEntry> MaskEntryRef;
    std::unordered_map<std::string,MaskEntryRef> maskEntriesByName;
    std::unordered_map<SimpleIdentity,MaskEntryRef> maskEntriesByID;
    // We have 32 bits of range in the mask ID on iOS
    unsigned int lastMaskID = 0;
    std::mutex maskLock;
};
typedef std::shared_ptr<ComponentManager> ComponentManagerRef;

// Make an OS specific component manager
extern ComponentManagerRef MakeComponentManager();
    
}
