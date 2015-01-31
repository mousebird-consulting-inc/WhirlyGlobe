/*
 *  LabelRenderer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/11/13.
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
#import <set>
#import <map>
#import "Identifiable.h"
#import "Drawable.h"
#import "TextureAtlas.h"
#import "FontTextureManager.h"
#import "LayoutManager.h"
#import "SelectionManager.h"
#import "Dictionary.h"
#import "Platform.h"

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
    LabelSceneRep();
    LabelSceneRep(SimpleIdentity theId) : Identifiable(theId) { }
    ~LabelSceneRep() { }
    
    float fade;          // Fade interval, for deletion
    SimpleIDSet texIDs;  // Textures we created for this
    SimpleIDSet drawIDs; // Drawables created for this
    SimpleIDSet drawStrIDs;  // Drawable strings created with the font manager
    SimpleIDSet layoutIDs;  // Screen space objects
    SimpleIDSet selectIDs;  // Selection rect
};
typedef std::set<LabelSceneRep *,IdentifiableSorter> LabelSceneRepSet;
    
// How a label is justified for display
typedef enum {WhirlyKitLabelMiddle,WhirlyKitLabelLeft,WhirlyKitLabelRight} LabelJustify;

// Label spec passed around between threads
class LabelInfo
{
public:
    LabelInfo();
    void parseDict(const Dictionary &dict);

    RGBAColor textColor,backColor;
    bool screenObject;
    bool layoutEngine;
    float layoutImportance;
    float width,height;
    int drawOffset;
    float minVis,maxVis;
    LabelJustify justify;
    int drawPriority;
    float fade;
    RGBAColor shadowColor;
    float shadowSize;
    RGBAColor outlineColor;
    float outlineSize;
    SimpleIdentity shaderID;
    bool enable;
    WhirlyKit::SimpleIdentity programID;
};
    
/** Used to render a group of labels, possibly on
    a dispatch queue.  Up to you to set that up.
    You call this and process the results.
  */
class LabelRenderer
{
public:
    LabelRenderer(Scene *scene,FontTextureManager *fontTexManager,const LabelInfo *labelInfo);
    
    /// Description of the labels
    const LabelInfo *labelInfo;
    /// How big texture atlases should be if we're not using fonts
    int textureAtlasSize;
    /// Coordinate system display adapater
    CoordSystemDisplayAdapter *coordAdapter;
    /// Label represention (return value)
    LabelSceneRep *labelRep;
    /// Scene we're building in
    Scene *scene;
    /// Screen space objects
    std::vector<WhirlyKit::ScreenSpaceObject> screenObjects;
    /// Layout objects (pass these to the layout engine if you want that)
    std::vector<LayoutObject> layoutObjects;
    /// Selectable objects (3D) to pass to the selection manager
    std::vector<RectSelectable3D> selectables3D;
    /// Selectable objects (2D) to pass to the selection manager
    std::vector<RectSelectable2D> selectables2D;

    /// Change requests to pass to the scene
    ChangeSet changeRequests;
    /// Font texture manager to use if we're doing fonts
    FontTextureManager *fontTexManager;
    /// Set if want to use attributed strings (we usually do)
    bool useAttributedString;
    /// Scale, if we're using that
    float scale;

    /// Renders the labels into a big texture and stores the resulting info
    void render(std::vector<SingleLabel *> &labels,ChangeSet &changes);
};

}
