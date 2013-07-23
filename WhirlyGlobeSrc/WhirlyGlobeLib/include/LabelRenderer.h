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
#import "DataLayer.h"
#import "LayerThread.h"
#import "TextureAtlas.h"
#import "SelectionManager.h"
#import "LayoutLayer.h"
#import "FontTextureManager.h"

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
    
    float fade;          // Fade interval, for deletion
    SimpleIDSet texIDs;  // Textures we created for this
    SimpleIDSet drawIDs; // Drawables created for this
    SimpleIDSet drawStrIDs;  // Drawable strings created with the font manager
    SimpleIDSet screenIDs;  // Screen space objects
    SimpleIdentity selectID;  // Selection rect
};
typedef std::set<LabelSceneRep *,IdentifiableSorter> LabelSceneRepSet;
    
}

// How a label is justified for display
typedef enum {WhirlyKitLabelMiddle,WhirlyKitLabelLeft,WhirlyKitLabelRight} WhirlyKitLabelJustify;

// Label spec passed around between threads
@interface WhirlyKitLabelInfo : NSObject
{
    NSArray                 *strs;  // SingleLabel objects
    UIColor                 *textColor;
    UIColor                 *backColor;
    UIFont                  *font;
    bool                    screenObject;
    bool                    layoutEngine;
    float                   layoutImportance;
    float                   width,height;
    int                     drawOffset;
    float                   minVis,maxVis;
    WhirlyKitLabelJustify            justify;
    int                     drawPriority;
    float                   fade;
    UIColor                 *shadowColor;
    float                   shadowSize;
    UIColor                 *outlineColor;
    float                   outlineSize;
}

@property (nonatomic) NSArray *strs;
@property (nonatomic) UIColor *textColor,*backColor;
@property (nonatomic) UIFont *font;
@property (nonatomic,assign) bool screenObject;
@property (nonatomic,assign) bool layoutEngine;
@property (nonatomic,assign) float layoutImportance;
@property (nonatomic,assign) float width,height;
@property (nonatomic,assign) int drawOffset;
@property (nonatomic,assign) float minVis,maxVis;
@property (nonatomic,assign) WhirlyKitLabelJustify justify;
@property (nonatomic,assign) int drawPriority;
@property (nonatomic,assign) float fade;
@property (nonatomic,strong) UIColor *shadowColor;
@property (nonatomic,assign) float shadowSize;
@property (nonatomic) UIColor *outlineColor;
@property (nonatomic,assign) float outlineSize;

- (id)initWithStrs:(NSArray *)inStrs desc:(NSDictionary *)desc;

@end

/** Used to render a group of labels, possibly on
    a dispatch queue.  Up to you to set that up.
  */
@interface WhirlyKitLabelRenderer : NSObject
{
@public
    WhirlyKitLabelInfo *labelInfo;
    int textureAtlasSize;
    WhirlyKit::CoordSystemDisplayAdapter *coordAdapter;
    WhirlyKit::LabelSceneRep *labelRep;
    WhirlyKit::Scene *scene;
    WhirlyKit::SimpleIdentity screenGenId;
    std::vector<WhirlyKit::LayoutObject> layoutObjects;
    std::vector<WhirlyKit::RectSelectable3D> selectables3D;
    std::vector<WhirlyKit::RectSelectable2D> selectables2D;
    
    std::vector<WhirlyKit::ChangeRequest *> changeRequests;
    WhirlyKitFontTextureManager *fontTexManager;
    bool useAttributedString;
}

/// Renders the labels into a big texture and stores the resulting info
- (void)render;

@end

