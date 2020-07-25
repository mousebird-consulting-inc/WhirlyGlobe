/*
 *  BillboardManager.h
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
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
#import <map>
#import "Identifiable.h"
#import "BasicDrawable.h"
#import "BillboardDrawableBuilder.h"
#import "Scene.h"
#import "SelectionManager.h"
#import "BaseInfo.h"

namespace WhirlyKit
{
    
/// Used to represent a single billboard polygon
///  with one texture
class SingleBillboardPoly
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    SingleBillboardPoly();
    ~SingleBillboardPoly(){};

    /// Coordinates of polygons
    Point2dVector pts;
    /// Texture coordinates to go with polygons
    std::vector<WhirlyKit::TexCoord> texCoords;
    /// Color of geometry
    RGBAColor color;
    /// Texture to use
    WhirlyKit::SimpleIdentity texId;
    /// Vertex attributes applied to just this poly
    SingleVertexAttributeSet vertexAttrs;
};
    
/** Single billboard representation.  Billboards are oriented towards
the user.  Fill this out and hand it over to the billboard layer
to manage.
*/
class Billboard
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    Billboard();
    ~Billboard(){};

    /// Center in display coordinates
    WhirlyKit::Point3d center;
    /// Polygons that make up this billboard (there can be more than one)
    std::vector<WhirlyKit::SingleBillboardPoly> polys;
    /// Size (for selection)
    WhirlyKit::Point2d size;
    /// If set, this marker should be made selectable
    ///  and it will be if the selection layer has been set
    bool isSelectable;
    /// Unique ID for selection
    WhirlyKit::SimpleIdentity selectID;

};

// Used to pass parameters around between threads
class BillboardInfo : public BaseInfo
{
public:
    BillboardInfo();
    BillboardInfo(const Dictionary &);
    ~BillboardInfo(){};
    
    typedef enum {Eye=0,Ground} Orient;
    Orient orient;
    RGBAColor color;
};
typedef std::shared_ptr<BillboardInfo> BillboardInfoRef;

/// Used internally to track billboard geometry
class BillboardSceneRep : public Identifiable
{
public:
    BillboardSceneRep();
    BillboardSceneRep(SimpleIdentity inId);
    ~BillboardSceneRep();

    // Clear the contents out of the scene
    void clearContents(SelectionManager *selectManager,ChangeSet &changes,TimeInterval when);

    SimpleIDSet drawIDs;  // Drawables created for this
    SimpleIDSet selectIDs;  // IDs used for selection
    float fade;  // Time to fade away for removal
};

typedef std::set<BillboardSceneRep *,IdentifiableSorter> BillboardSceneRepSet;

// Used to construct billboard geometry
class BillboardBuilder
{
public:
    BillboardBuilder(Scene *scene,SceneRenderer *sceneRender,ChangeSet &changes,BillboardSceneRep *sceneRep,const BillboardInfo &billInfo,SimpleIdentity billboardProgram,SimpleIdentity texId);
    ~BillboardBuilder();

    void addBillboard(Point3d center,const Point2dVector &pts,const std::vector<WhirlyKit::TexCoord> &texCoords, const RGBAColor *inColor,const SingleVertexAttributeSet &vertAttrs);

    void flush();

protected:
    Scene *scene;
    SceneRenderer *sceneRender;
    ChangeSet &changes;
    Mbr drawMbr;
    BillboardDrawableBuilderRef drawable;
    const BillboardInfo &billInfo;
    BillboardSceneRep *sceneRep;
    SimpleIdentity billboardProgram;
    SimpleIdentity texId;
};
typedef std::shared_ptr<BillboardBuilder> BillboardBuilderRef;

#define kWKBillboardManager "WKBillboardManager"

/** The Billboard Manager handles billboard related geometry.
This object is thread safe except for deletion.
*/
class BillboardManager : public SceneManager
{
public:
    BillboardManager();
    virtual ~BillboardManager();

    /// Add billboards for display
    SimpleIdentity addBillboards(std::vector<Billboard*> billboards,const BillboardInfo &billboardInfo,ChangeSet &changes);

    /// Enable/disable active billboards
    void enableBillboards(SimpleIDSet &billIDs,bool enable,ChangeSet &changes);

    /// Remove a group of billboards named by the given ID
    void removeBillboards(SimpleIDSet &billIDs,ChangeSet &changes);

protected:
    std::mutex billLock;
    BillboardSceneRepSet sceneReps;
};

}
