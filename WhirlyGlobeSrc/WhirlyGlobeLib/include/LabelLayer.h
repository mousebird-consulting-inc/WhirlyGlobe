/*
 *  LabelLayer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/7/11.
 *  Copyright 2011-2012 mousebird consulting
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
#import "DrawCost.h"
#import "SelectionLayer.h"
#import "LayoutLayer.h"

namespace WhirlyKit 
{

/// Default for label draw priority
static const int LabelDrawPriority=1000;

/** The Label Scene Representation is used to encapsulate a set of
    labels that are being added or have been added to the scene and
    their associated textures and drawable IDs.
  */
class LabelSceneRep : public Identifiable
{
public:
    LabelSceneRep();
    ~LabelSceneRep() { }
    
    float fade;          // Fade interval, for deletion
    SimpleIDSet texIDs;  // Textures we created for this
    SimpleIDSet drawIDs; // Drawables created for this
    SimpleIDSet screenIDs;  // Screen space objects
    SimpleIDSet selectIDs;  // Selection rects
};
typedef std::map<SimpleIdentity,LabelSceneRep *> LabelSceneRepMap;

}

/** The Single Label represents one label with its text, location,
    and an NSDictionary that can be used to override some attributes.
    In general we don't want to create just one label, we want to
    create a large number of labels at once.  We use an array of
    these single labels to do that.
  */
@interface WhirlyKitSingleLabel : NSObject
{
    /// If set, this marker should be made selectable
    ///  and it will be if the selection layer has been set
    bool isSelectable;
    /// If the marker is selectable, this is the unique identifier
    ///  for it.  You should set this ahead of time
    WhirlyKit::SimpleIdentity selectID;
    /// The text we want to see
    NSString *text;
    /// A geolocation for the middle, left or right of the label
    ///  depending on the justification
    WhirlyKit::GeoCoord loc;
    /// This dictionary contains overrides for certain attributes
    ///  for just this label.  Only width, height, icon, text color, and
    ///  background color supported.
    NSDictionary *desc;
    /// If non-zero, this is the texture to use as an icon
    WhirlyKit::SimpleIdentity iconTexture;
    /// If set, this moves the label if displayed in screen (2D) mode
    CGSize screenOffset;
}

@property (nonatomic,assign) bool isSelectable;
@property (nonatomic,assign) WhirlyKit::SimpleIdentity selectID;
@property (nonatomic,retain) NSString *text;
@property (nonatomic,assign) WhirlyKit::GeoCoord loc;
@property (nonatomic,retain) NSDictionary *desc;
@property (nonatomic,assign) WhirlyKit::SimpleIdentity iconTexture;
@property (nonatomic,assign) CGSize screenOffset;

/// This is used to sort out width and height from the defaults.  Pass
///  in the value of one and zero for the other and it will fill in the
///  missing one.
- (bool)calcWidth:(float *)width height:(float *)height defaultFont:(UIFont *)font;

/// This will calculate the real extents in 3D over the globe.
/// Pass in an array of 3 point3f structures for the points and
///  normals.  The corners are returned in counter-clockwise order.
/// This is used for label selection
- (void)calcExtents:(NSDictionary *)topDesc corners:(WhirlyKit::Point3f *)pts norm:(WhirlyKit::Point3f *)norm coordAdapter:(WhirlyKit::CoordSystemDisplayAdapter *)coordAdapter;

@end

namespace WhirlyKit
{

/// Size of one side of the texture atlases built for labels
/// You can also specify this at startup
static const unsigned int LabelTextureAtlasSizeDefault = 512;
    
}

/** The Label Layer will represent and manage groups of labels.  You
    can hand it a list of labels to display and it will group those
    in to associated drawables.  You want to give it a group for speed.
    Labels are currently drawn in Quartz, compiled into texture atlases
    and drawn together.  This will change in the future to use font
    textures.
 
    When you add a group of labels you will get back a unique ID
    which can be used to modify or delete all those labels at once.
 
    The label display can be controlled via the individual SingleLabel
    objects as well as overall look and feel with the label description
    dictionary.  That dictionary can contain the following:
 
    <list type="bullet">
    <item>enable          [NSNumber bool]
    <item>drawOffset      [NSNumber int]
    <item>drawPriority    [NSNumber int]
    <item>label           [NSString]
    <item>textColor       [UIColor]
    <item>backgroundColor [UIColor]
    <item>font            [UIFont]
    <item>width           [NSNumber float]  [In display coordinates, not geo]
    <item>height          [NSNumber float]
    <item>minVis          [NSNumber float]
    <item>maxVis          [NSNumber float]
    <item>justify         [NSString>] middle, left, right
    <item>fade            [NSSTring float]
    <item>screen          [NSNumber bool]  [If true, this is a 2D object, width and height are in screen coordinates]
    <item>layout          [NSNumber bool]  [If true, pass this off to the layout engine to compete with other labels]
    <item>layoutImportance [NSNumber float]  [If set and layout is on, this is the importance value used in competition in the layout layer]
    <item>shadowSize      [NSNumber float]  [If set, we'll draw a background shadow underneath the text of this width]
    <item>shadowColor     [UIcolor]  [If shadow size is non-zero, this will be the color we draw the shadow in.  Defaults to black.]
    </list>
  */
@interface WhirlyKitLabelLayer : NSObject<WhirlyKitLayer>
{
	WhirlyKitLayerThread * __weak layerThread;
	WhirlyKit::Scene *scene;
    
    /// Screen space generator on the render side
    WhirlyKit::SimpleIdentity screenGenId;
    
    /// If set, we register labels as selectable here
    WhirlyKitSelectionLayer * __weak selectLayer;

    /// If set, this is the layout layer we'll pass some labels off to (those being laid out)
    WhirlyKitLayoutLayer * __weak layoutLayer;

    /// Keep track of labels (or groups of labels) by ID for deletion
    WhirlyKit::LabelSceneRepMap labelReps;
    
    unsigned int textureAtlasSize;
}

/// Set this to enable selection for labels
@property (nonatomic,weak) WhirlyKitSelectionLayer *selectLayer;

/// Set this to use the layout engine for labels so marked
@property (nonatomic,weak) WhirlyKitLayoutLayer *layoutLayer;

/// Initialize the label layer with a size for texture atlases
/// Needs to be a power of 2
- (id)initWithTexAtlasSize:(unsigned int)textureAtlasSize;

/// Called in the layer thread
- (void)startWithThread:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene;

/// Called by the layer thread
- (void)shutdown;

/// Create a label at the given coordinates, with the given look and feel.
/// You get an ID for the label back so you can delete or modify it later.
/// If you have more than one label, call addLabels instead.
- (WhirlyKit::SimpleIdentity) addLabel:(NSString *)str loc:(WhirlyKit::GeoCoord)loc desc:(NSDictionary *)desc;

/// Add a single label with the SingleLabel object.  The desc dictionary in that
///  object will specify the look
- (WhirlyKit::SimpleIdentity) addLabel:(WhirlyKitSingleLabel *)label;

/// Add a whole list of labels (represented by SingleLabel objects) with the given
///  look and feel.
/// You get the ID identifying the whole group for modification or deletion
- (WhirlyKit::SimpleIdentity) addLabels:(NSArray *)labels desc:(NSDictionary *)desc;

/// Change the display of a given label accordingly to the desc dictionary.
/// Only minVis and maxVis are supported
- (void)changeLabel:(WhirlyKit::SimpleIdentity)labelID desc:(NSDictionary *)dict;

/// Return the cost of a given label group (number of drawables and textures).
/// Only call this in the layer thread
- (WhirlyKitDrawCost *)getCost:(WhirlyKit::SimpleIdentity)labelID;

/// Remove the given label group by ID
- (void) removeLabel:(WhirlyKit::SimpleIdentity)labelId;

@end
