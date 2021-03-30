 /*
 *  VectorManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/22/13.
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
    VectorSceneRep() : fade(0.0) { }
    VectorSceneRep(SimpleIdentity theId) : Identifiable(theId) { }
    
    // Clean out the representation
    void clear(ChangeSet &changes);
    
    SimpleIDSet drawIDs;    // The drawables we created
    SimpleIDSet instIDs;    // Instances if we're doing that
    float fade;       // If set, the amount of time to fade out before deletion
};
typedef std::set<VectorSceneRep *,IdentifiableSorter> VectorSceneRepSet;

typedef enum {TextureProjectionNone,TextureProjectionTanPlane,TextureProjectionScreen} TextureProjections;

// Used to describe the drawable we'll construct for a given vector
class VectorInfo : public BaseInfo
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    VectorInfo();
    VectorInfo(const Dictionary &dict);
    virtual ~VectorInfo() = default;

    // Convert contents to a string for debugging
    virtual std::string toString();
    
    bool                        filled;
    float                       sample;
    SimpleIdentity              texId;
    Point2f                     texScale;
    float                       subdivEps;
    bool                        gridSubdiv;
    TextureProjections          texProj;
    RGBAColor                   color;
    float                       lineWidth;
    bool                        centered;
    bool                        vecCenterSet;
    Point2f                     vecCenter;
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
    VectorManager();
    virtual ~VectorManager();
    
    /// Add an array of vectors.  The returned ID can be used for removal.
    SimpleIdentity addVectors(ShapeSet *shapes,const VectorInfo &desc,ChangeSet &changes);
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
