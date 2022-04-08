 /*
 *  VectorManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/22/13.
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

#import <math.h>
#import <vector>
#import <set>
#import <map>
#import "SceneRenderer.h"
#import "BasicDrawableBuilder.h"
#import "BasicDrawableInstanceBuilder.h"
#import "VectorData.h"
#import "GlobeMath.h"
#import "Dictionary.h"
#import "Scene.h"
#import "BaseInfo.h"

namespace WhirlyKit
{

/*  This is the representation of a group of vectors
     in the scene.  You do not want to create individual
     vector features on the globe one by one, that's too expensive.
     It needs to be batched and that's how we do this.
     The VectorSceneRep keeps track of what shapes are being
     represented by what drawables in the scene.  We use this
     for modifications or deletions later on.
 */
class VectorSceneRep : public Identifiable
{
public:
    VectorSceneRep() = default;
    VectorSceneRep(SimpleIdentity theId) : Identifiable(theId) { }
    
    // Clean out the representation
    void clear(ChangeSet &changes);
    
    SimpleIDSet drawIDs;    // The drawables we created
    SimpleIDSet instIDs;    // Instances if we're doing that
    float fadeOut = 0.0;       // If set, the amount of time to fade out before deletion
};
typedef std::set<VectorSceneRep *,IdentifiableSorter> VectorSceneRepSet;

typedef enum {TextureProjectionNone,TextureProjectionTanPlane,TextureProjectionScreen} TextureProjections;

// Used to describe the drawable we'll construct for a given vector
class VectorInfo : public BaseInfo
{
public:
    VectorInfo() = default;
    VectorInfo(const Dictionary &dict);
    virtual ~VectorInfo() = default;

    // Convert contents to a string for debugging
    virtual std::string toString() const override;

    bool                        filled = false;
    float                       sample = 0.0f;
    SimpleIdentity              texId = EmptyIdentity;
    Point2f                     texScale = { 1.0f, 1.0f };
    float                       subdivEps = 0.0f;
    bool                        gridSubdiv = false;
    TextureProjections          texProj = TextureProjectionNone;
    RGBAColor                   color = RGBAColor::white();
    float                       lineWidth = 1.0f;
    bool                        centered = true;
    bool                        vecCenterSet = false;
    bool                        closeAreals = true;
    bool                        selectable = true;
    Point2f                     vecCenter = { 0.0f, 0.0f };
    FloatExpressionInfoRef      opacityExp;
    ColorExpressionInfoRef      colorExp;
};
typedef std::shared_ptr<VectorInfo> VectorInfoRef;

#define kWKVectorManager "WKVectorManager"

/** The Vector Manager is used to create and destroy geometry associated with
    vector display.  It's entirely thread safe (except for destruction).
  */
class VectorManager : public SceneManager
{
public:
    VectorManager() = default;
    virtual ~VectorManager();
    
    /// Add an array of vectors.  The returned ID can be used for removal.
    SimpleIdentity addVectors(const std::vector<VectorShapeRef> &shapes,const VectorInfo &desc,ChangeSet &changes);
    SimpleIdentity addVectors(const ShapeSet *shapes,const VectorInfo &desc,ChangeSet &changes);
    SimpleIdentity addVectors(const std::vector<VectorShapeRef> *shapes,const VectorInfo &desc,ChangeSet &changes);

    /// Change the vector(s) represented by the given ID
    void changeVectors(SimpleIdentity vecID,const VectorInfo &vecInfo,ChangeSet &changes);
    
    /// Make an instance of the given vectors with the given attributes and return an ID to identify them.
    SimpleIdentity instanceVectors(SimpleIdentity vecID,const VectorInfo &vecInfo,ChangeSet &changes);

    /// Remove a group of vectors associated with the given ID
    void removeVectors(SimpleIDSet &vecIDs,ChangeSet &changes);
    
    /// Enable/disable vector data
    void enableVectors(SimpleIDSet &vecIDs,bool enable,ChangeSet &changes);
    
protected:
    VectorSceneRepSet vectorReps;
};
typedef std::shared_ptr<VectorManager> VectorManagerRef;

}
