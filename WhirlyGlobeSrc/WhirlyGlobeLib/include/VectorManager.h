 /*
 *  VectorManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/22/13.
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
#import <vector>
#import <set>
#import <map>
#import "Drawable.h"
#import "VectorData.h"
#import "GlobeMath.h"
#import "Dictionary.h"
#import "Scene.h"

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

typedef enum {TextureProjectionNone,TextureProjectionTanPlane} TextureProjections;

// Used to describe the drawable we'll construct for a given vector
class VectorInfo
{
public:
    VectorInfo();
    void parseDict(const Dictionary &dict);
    
    bool                        enable;
    float                       drawOffset;
    int                         priority;
    float                       minVis,maxVis;
    bool                        filled;
    float                       sample;
    SimpleIdentity              texId;
    Point2f                     texScale;
    float                       subdivEps;
    bool                        gridSubdiv;
    TextureProjections          texProj;
    RGBAColor                   color;
    float                       fade;
    float                       lineWidth;
    bool                        centered;
};

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
    
    /// Change the vector(s) represented by the given ID
    void changeVectors(SimpleIdentity vecID,const Dictionary *desc,ChangeSet &changes);
    
    /// Make an instance of the give vectors with the given attributes and return an ID to identify them.
    SimpleIdentity instanceVectors(SimpleIdentity vecID,const Dictionary *desc,ChangeSet &changes);

    /// Remove a group of vectors associated with the given ID
    void removeVectors(SimpleIDSet &vecIDs,ChangeSet &changes);
    
    /// Enable/disable vector data
    void enableVectors(SimpleIDSet &vecIDs,bool enable,ChangeSet &changes);
    
protected:
    pthread_mutex_t vectorLock;
    VectorSceneRepSet vectorReps;
};

}
