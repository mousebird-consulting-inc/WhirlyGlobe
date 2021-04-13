/*
 *  LabelRenderer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/11/13.
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
#import "TextureAtlas.h"
#import "FontTextureManager.h"
#import "LayoutManager.h"
#import "SelectionManager.h"
#import "Dictionary.h"
#import "Platform.h"
#import "BaseInfo.h"

namespace WhirlyKit
{
    
/// Default for label draw priority
static const int LabelDrawPriority=1000;

/// Size of one side of the texture atlases built for labels
/// You can also specify this at startup
static const unsigned int LabelTextureAtlasSizeDefault = 512;
    
class SingleLabel;
    
/** The Label Scene Representation is used to encapsulate a set of
    labels that are being added or have been added to the scene and
    their associated textures and drawable IDs.
 */
class LabelSceneRep : public Identifiable
{
public:
    LabelSceneRep() { }
    LabelSceneRep(SimpleIdentity theId) : Identifiable(theId) { }

    float fadeOut;          // Fade interval, for deletion
    SimpleIDSet texIDs;  // Textures we created for this
    SimpleIDSet drawIDs; // Drawables created for this
    SimpleIDSet drawStrIDs;  // Drawable strings created with the font manager
    SimpleIDSet layoutIDs;  // Screen space objects
    SimpleIDSet selectIDs;  // Selection rect
};
typedef std::set<LabelSceneRep *,IdentifiableSorter> LabelSceneRepSet;
    
// How a label is justified for display
typedef enum {WhirlyKitLabelMiddle,WhirlyKitLabelLeft,WhirlyKitLabelRight} LabelJustify;
typedef enum {WhirlyKitTextCenter,WhirlyKitTextLeft,WhirlyKitTextRight} TextJustify;

// Label spec passed around between threads
class LabelInfo : public BaseInfo
{
public:
    LabelInfo(bool screenObject);
    LabelInfo(const LabelInfo &that);
    LabelInfo(const Dictionary &dict,bool screenObject);
    virtual ~LabelInfo() = default;

    bool hasTextColor = false;
    RGBAColor textColor = RGBAColor::white();
    RGBAColor backColor = RGBAColor::clear();
    bool screenObject  = true;
    float width = 0.0f;
    float height = 0.0f;
    LabelJustify labelJustify = WhirlyKitLabelMiddle;
    TextJustify textJustify = WhirlyKitTextCenter;
    RGBAColor shadowColor = RGBAColor::black();
    float shadowSize = -1.0f;
    RGBAColor outlineColor = RGBAColor::black();
    float outlineSize = -1.0f;
    float lineHeight = 0.0f;
    float fontPointSize = 16.0f;
    float layoutOffset = 0.0f;
    float layoutSpacing = 20.0f;
    int layoutRepeat = 0;
    bool layoutDebug = false;

    FloatExpressionInfoRef opacityExp;
//    ColorExpressionInfoRef colorExp;
    FloatExpressionInfoRef scaleExp;
};

typedef std::shared_ptr<LabelInfo> LabelInfoRef;
    
/** Used to render a group of labels, possibly on
    a dispatch queue.  Up to you to set that up.
    You call this and process the results.
  */
class LabelRenderer
{
public:
    LabelRenderer(Scene *scene,
                  SceneRenderer *renderer,
                  FontTextureManagerRef fontTexManager,
                  const LabelInfo *labelInfo,
                  SimpleIdentity maskProgID);

    /// Description of the labels
    const LabelInfo *labelInfo = nullptr;
    /// How big texture atlases should be if we're not using fonts
    int textureAtlasSize = 2048;
    /// Coordinate system display adapter
    CoordSystemDisplayAdapter *coordAdapter = nullptr;
    /// Label representation (return value)
    LabelSceneRep *labelRep = nullptr;
    /// Scene we're building in
    Scene *scene = nullptr;
    SceneRenderer *renderer = nullptr;
    /// Screen space objects
    std::vector<WhirlyKit::ScreenSpaceObject> screenObjects;
    /// Layout objects (pass these to the layout engine if you want that)
    std::vector<LayoutObject> layoutObjects;
    /// Selectable objects (3D) to pass to the selection manager
    std::vector<RectSelectable3D> selectables3D;
    /// Selectable objects (2D) to pass to the selection manager
    std::vector<RectSelectable2D> selectables2D;
    /// Moving selectable objects (2D) to pass to the selection manager
    std::vector<MovingRectSelectable2D> movingSelectables2D;

    /// Change requests to pass to the scene
    ChangeSet changeRequests;
    /// Font texture manager to use if we're doing fonts
    FontTextureManagerRef fontTexManager;
    /// Set if want to use attributed strings (we usually do)
    bool useAttributedString = true;
    /// Scale, if we're using that
    float scale = 1.0f;
    // Program used to render masks to their target
    SimpleIdentity maskProgID = 0;
    
    /// Convenience routine to convert the points to model space
    Point3dVector convertGeoPtsToModelSpace(const VectorRing &inPts);

    /// Renders the labels into a big texture and stores the resulting info
    void render(PlatformThreadInfo *threadInfo,const std::vector<SingleLabel *> &labels,ChangeSet &changes);

    /// Renders the labels into a big texture and stores the resulting info
    void render(PlatformThreadInfo *threadInfo,
                const std::vector<SingleLabel *> &labels,
                ChangeSet &changes,
                const std::function<bool(PlatformThreadInfo*)>& cancelFn);
};

}
