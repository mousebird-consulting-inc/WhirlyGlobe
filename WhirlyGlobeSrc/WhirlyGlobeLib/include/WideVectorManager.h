/*
 *  WideVectorManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/29/14.
 *  Copyright 2011-2015 mousebird consulting.
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
#import "BasicDrawableInstance.h"
#import "WideVectorDrawable.h"
#import "Scene.h"
#import "SelectionManager.h"
#import "VectorData.h"
#import "Dictionary.h"
#import "BaseInfo.h"

namespace WhirlyKit
{

/// Vectors are widened in real world or screen coordinates
typedef enum {WideVecCoordReal,WideVecCoordScreen} WideVectorCoordsType;

/// How the lines are joined.  See: http://www.w3.org/TR/SVG/painting.html#StrokeLinejoinProperty
typedef enum {WideVecMiterJoin,WideVecRoundJoin,WideVecBevelJoin} WideVectorLineJoinType;
    
/// How the lines begin and end.  See: http://www.w3.org/TR/SVG/painting.html#StrokeLinecapProperty
typedef enum {WideVecButtCap,WideVecRoundCap,WideVecSquareCap} WideVectorLineCapType;
    
/** Used to pass parameters for the wide vectors around.
  */
class WideVectorInfo : public BaseInfo
{
public:
    WideVectorInfo();
    WideVectorInfo(const Dictionary &dict);

    RGBAColor color;
    float width;
    float repeatSize;
    float edgeSize;
    WideVectorCoordsType coordType;
    WideVectorLineJoinType joinType;
    WideVectorLineCapType capType;
    SimpleIdentity texID;
    float miterLimit;
};
    
/// Used to track the
class WideVectorSceneRep : public Identifiable
{
public:
    WideVectorSceneRep();
    WideVectorSceneRep(SimpleIdentity inId);
    ~WideVectorSceneRep();
    
    void enableContents(bool enable,ChangeSet &changes);
    void clearContents(ChangeSet &changes);
    
    SimpleIDSet drawIDs;
    SimpleIDSet instIDs;    // Instances if we're doing that
    float fade;
};

typedef std::set<WideVectorSceneRep *,IdentifiableSorter> WideVectorSceneRepSet;

#define kWKWideVectorManager "WKWideVectorManager"

/** The Wide Vector Manager handles linear features that we widen into
    polygons and display in real world or screen size.
  */
class WideVectorManager : public SceneManager
{
public:
    WideVectorManager();
    virtual ~WideVectorManager();

    /// Add widened vectors for display
    SimpleIdentity addVectors(ShapeSet *shapes,const WideVectorInfo &desc,ChangeSet &changes);

    /// Enable/disable active vectors
    void enableVectors(SimpleIDSet &vecIDs,bool enable,ChangeSet &changes);
    
    /// Make an instance of the give vectors with the given attributes and return an ID to identify them.
    SimpleIdentity instanceVectors(SimpleIdentity vecID,const WideVectorInfo &desc,ChangeSet &changes);
    
    /// Remove a gruop of vectors named by the given ID
    void removeVectors(SimpleIDSet &vecIDs,ChangeSet &changes);
    
protected:
    pthread_mutex_t vecLock;
    WideVectorSceneRepSet sceneReps;
};
    
}
