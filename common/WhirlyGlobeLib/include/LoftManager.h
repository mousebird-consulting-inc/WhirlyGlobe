/*
 *  LoftManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/30/13.
 *  Copyright 2011-2022 mousebird consulting.
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

#import "Identifiable.h"
#import "BaseInfo.h"
#import "Dictionary.h"
#import "Scene.h"
#import "BasicDrawable.h"
#import "SelectionManager.h"
#import "VectorData.h"
#import "SharedAttributes.h"

#import <math.h>
#import <set>
#import <map>

namespace WhirlyKit
{
    
// Used to describe the drawables we want to construct for a given vector
struct LoftedPolyInfo : public BaseInfo
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    LoftedPolyInfo();
    LoftedPolyInfo(const Dictionary &dict);
    virtual ~LoftedPolyInfo() = default;

    float       height = 0.01f;
    float       base = 0.0;
    bool        top = true;
    bool        side = true;
    bool        layered = false;
    bool        outline = true;
    bool        outlineSide = false;
    bool        outlineBottom = false;
    int         outlineDrawPriority = MaplyLoftedPolysDrawPriorityDefault+1;
    RGBAColor   color = RGBAColor::white();
    RGBAColor   outlineColor = RGBAColor::white();
    float       outlineWidth = 1.0f;
    bool        centered = false;
    bool        hasCenter = false;
    Point2d     center = { 0, 0 };
    double      gridSize = 10.0 / 180.0 * M_PI;
};
typedef std::shared_ptr<LoftedPolyInfo> LoftedPolyInfoRef;

/** Representation of one or more lofted polygons.
 Used to keep track of the assets we create.
 */
struct LoftedPolySceneRep : public WhirlyKit::Identifiable
{
    LoftedPolySceneRep() = default;
    LoftedPolySceneRep(SimpleIdentity theId) : Identifiable(theId) { }
    
    WhirlyKit::SimpleIDSet drawIDs;  // Drawables created for this
    float fadeOut = 0.0;            // Fade out, used for delete
};
typedef std::set<LoftedPolySceneRep *,IdentifiableSorter> LoftedPolySceneRepSet;

#define kWKLoftedPolyManager "kWKLoftedPolyManager"
    
/** The Loft Manager handles the geometr associated with lofted polygons.
    It's entirely thread safe (except for destruction).
  */
struct LoftManager : public SceneManager
{
    LoftManager() = default;
    virtual ~LoftManager();

    /// Add lofted polygons
    SimpleIdentity addLoftedPolys(WhirlyKit::ShapeSet *shapes,const LoftedPolyInfo &polyInfo,ChangeSet &changes);

    /// Enable/disable lofted polys
    void enableLoftedPolys(const SimpleIDSet &polyIDs,bool enable,ChangeSet &changes);
    
    /// Remove lofted polygons
    void removeLoftedPolys(const SimpleIDSet &polyIDs,ChangeSet &changes);
        
protected:
    void addGeometryToBuilder(LoftedPolySceneRep *sceneRep,const LoftedPolyInfo &polyInfo,GeoMbr &drawMbr,Point3d &center,bool centerValid,Point2d &geoCenter,ShapeSet &shapes, VectorTrianglesRef triMesh,std::vector<WhirlyKit::VectorRing> &outlines,ChangeSet &changes);
    
    LoftedPolySceneRepSet loftReps;
};
typedef std::shared_ptr<LoftManager> LoftManagerRef;

}
