/*
 *  LabelManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/22/13.
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
#import "TextureAtlas.h"
#import "SelectionManager.h"
#import "LayoutManager.h"
#import "LabelRenderer.h"

namespace WhirlyKit
{
    
/** The Single Label represents one label with its text, location,
 and a Dictionary that can be used to override some attributes.
 In general we don't want to create just one label, we want to
 create a large number of labels at once.  We use an array of
 these single labels to do that.
 */
class SingleLabel
{
public:
    SingleLabel();
    virtual ~SingleLabel() { };

    /// If set, this marker should be made selectable
    ///  and it will be if the selection layer has been set
    bool isSelectable;
    /// If the marker is selectable, this is the unique identifier
    ///  for it.  You should set this ahead of time
    SimpleIdentity selectID;
    /// The text we want to see
//    NSString *text;
    std::string text;
    // Sometimes rather than strings, we pass around the code points
    std::vector<int> codePoints;
    
    /// A geolocation for the middle, left or right of the label
    ///  depending on the justification
    GeoCoord loc;
    /// Set if we're moving these over time (screen only)
    bool hasMotion;
    /// Set for animation over time
    GeoCoord endLoc;
    /// Timing for animation, if present
    TimeInterval startTime,endTime;
    /// Rotation around the origin
    float rotation;
    /// Keep a label oriented upright on the screen
    bool keepUpright;
    /// This dictionary contains overrides for certain attributes
    ///  for just this label.  Only width, height, icon, text color, and
    ///  background color supported.
    Dictionary desc;
    /// If non-zero, this is the texture to use as an icon
    SimpleIdentity iconTexture;
    /// If the texture is set and this is non-zero the size of the image
    Point2f iconSize;
    /// If set, this moves the label if displayed in screen (2D) mode
    Point2d screenOffset;

    // Used to build the drawable string on specific platforms
    virtual DrawableString *generateDrawableString(const LabelInfo *,FontTextureManager *fontTexManager,ChangeSet &changes) = 0;
    
// Note: Porting
//    /// This is used to sort out width and height from the defaults.  Pass
//    ///  in the value of one and zero for the other and it will fill in the
//    ///  missing one.
//    bool calcWidth(float *width,float *height,UIFont *font);
//
//    /// This will calculate the real extents in 3D over the globe.
//    /// Pass in an array of 3 point3f structures for the points and
//    ///  normals.  The corners are returned in counter-clockwise order.
//    /// This is used for label selection
//    void calcExtents(Dictionary *topDesc,Point3f *pts,Point3f *norm,CoordSystemDisplayAdapter *coordAdapter);
//
//    /// Slightly more specific version
//    void calcExtents2(float width2,float height2,const Point2f iconSize,WhirlyKitLabelJustify justify,Point3f *pts,Point3f *norm,Point3f *iconPts,CoordSystemDisplayAdapter *coordAdapter);
//
//    /// This version is for screen space labels
//    - (void)calcScreenExtents2:(float)width2 height2:(float)height2 iconSize:(WhirlyKit::Point2f)iconSize justify:(WhirlyKitLabelJustify)justify corners:(WhirlyKit::Point3f *)pts iconCorners:(WhirlyKit::Point3f *)iconPts useIconOffset:(bool)useIconOffset;
};
    
#define kWKLabelManager "WKLabelManager"

/** The label manager controls resources for text labels, including construction
    and destruction.  All methods (other than the destructor) are thread safe.
  */
class LabelManager : public SceneManager
{
public:
    LabelManager();
    virtual ~LabelManager();

    /// Add the given set of labels, returning an ID that represents the whole thing
    SimpleIdentity addLabels(std::vector<SingleLabel *> &labels,const LabelInfo &desc,ChangeSet &changes);
    
    /// Change visual attributes (just the visibility range)
//    void changeLabel(SimpleIdentity labelID,NSDictionary *desc,ChangeSet &changes);
    
    /// Remove the given label(s)
    void removeLabels(SimpleIDSet &labelID,ChangeSet &changes);
    
    /// Enable/disable labels
    void enableLabels(SimpleIDSet labelID,bool enable,ChangeSet &changes);
    
protected:
    pthread_mutex_t labelLock;
    
    /// Keep track of labels (or groups of labels) by ID for deletion
    WhirlyKit::LabelSceneRepSet labelReps;
    unsigned int textureAtlasSize;
};

}
