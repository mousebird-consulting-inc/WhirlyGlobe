/*
 *  LabelLayer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/7/11.
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
#import "LabelRenderer.h"
#import "LabelManager.h"

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
    <item>shader          [NSNumber long long]  [If set, the shader ID to use when drawing these labels.]
    </list>
  */
@interface WhirlyKitLabelLayer : NSObject<WhirlyKitLayer>

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

/// Remove the given label group by ID
- (void) removeLabel:(WhirlyKit::SimpleIdentity)labelId;

@end
