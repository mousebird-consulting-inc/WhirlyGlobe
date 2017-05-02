/*
 *  LabelRenderer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/11/13.
 *  Copyright 2011-2015 mousebird consulting
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
#import "DataLayer.h"
#import "LayerThread.h"
#import "TextureAtlas.h"
#import "SelectionManager.h"
#import "LayoutLayer.h"
#import "FontTextureManager.h"
#import "BaseInfo.h"

namespace WhirlyKit
{
    
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
    
    float fadeOut;          // Fade interval, for deletion
    SimpleIDSet texIDs;  // Textures we created for this
    SimpleIDSet drawIDs; // Drawables created for this
    SimpleIDSet drawStrIDs;  // Drawable strings created with the font manager
    SimpleIDSet layoutIDs;  // IDs passed to layout manager
    SimpleIDSet selectIDs;  // IDS passed to selection manager
};
typedef std::set<LabelSceneRep *,IdentifiableSorter> LabelSceneRepSet;
    
}

// How a label is justified for display
typedef enum {WhirlyKitLabelMiddle,WhirlyKitLabelLeft,WhirlyKitLabelRight} WhirlyKitLabelJustify;
typedef enum {WhirlyKitTextCenter,WhirlyKitTextLeft,WhirlyKitTextRight} WhirlyKitTextJustify;

// Label spec passed around between threads
@interface WhirlyKitLabelInfo : WhirlyKitBaseInfo

@property (nonatomic) UIColor *textColor,*backColor;
@property (nonatomic) UIFont *font;
@property (nonatomic,assign) bool screenObject;
@property (nonatomic,assign) bool layoutEngine;
@property (nonatomic,assign) float layoutImportance;
@property (nonatomic,assign) float width,height;
@property (nonatomic,assign) WhirlyKitLabelJustify labelJustify;
@property (nonatomic,assign) WhirlyKitTextJustify textJustify;
@property (nonatomic,strong) UIColor *shadowColor;
@property (nonatomic,assign) float shadowSize;
@property (nonatomic) UIColor *outlineColor;
@property (nonatomic,assign) float outlineSize;

- (id)initWithDesc:(NSDictionary *)desc;

@end

/** Used to render a group of labels, possibly on
    a dispatch queue.  Up to you to set that up.
    You call this and process the results.
  */
@interface WhirlyKitLabelRenderer : NSObject

/// Description of the labels
@property (nonatomic) WhirlyKitLabelInfo *labelInfo;
/// How big texture atlases should be if we're not using fonts
@property (nonatomic,assign) int textureAtlasSize;
/// Coordinate system display adapater
@property (nonatomic,assign) WhirlyKit::CoordSystemDisplayAdapter *coordAdapter;
/// Label represention (return value)
@property (nonatomic) WhirlyKit::LabelSceneRep *labelRep;
/// Scene we're building in
@property (nonatomic,assign) WhirlyKit::Scene *scene;
/// Screen space objects
@property (nonatomic,assign) std::vector<WhirlyKit::ScreenSpaceObject *> &screenObjects;
/// Layout objects (pass these to the layout engine if you want that)
@property (nonatomic,assign) std::vector<WhirlyKit::LayoutObject *> &layoutObjects;
/// Selectable objects (3D) to pass to the selection manager
@property (nonatomic,assign) std::vector<WhirlyKit::RectSelectable3D> &selectables3D;
/// Selectable objects (2D) to pass to the selection manager
@property (nonatomic,assign) std::vector<WhirlyKit::RectSelectable2D> &selectables2D;
/// Moving selectable objects (2D) to pass to the selection manager
@property (nonatomic,assign) std::vector<WhirlyKit::MovingRectSelectable2D> &movingSelectables2D;

/// Change requests to pass to the scene
@property (nonatomic,assign) std::vector<WhirlyKit::ChangeRequest *> &changeRequests;
/// Font texture manager to use if we're doing fonts
@property (nonatomic) WhirlyKitFontTextureManager *fontTexManager;
/// Set if want to use attributed strings (we usually do)
@property (nonatomic,assign) bool useAttributedString;
/// Scale, if we're using that
@property (nonatomic,assign) float scale;

/// Strings to render
@property (nonatomic) NSArray *strs;

/// Renders the labels into a big texture and stores the resulting info
- (void)render;

@end

