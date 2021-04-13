/*  LabelManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/22/13.
 *  Copyright 2011-2021 mousebird consulting
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
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    SingleLabel();
    virtual ~SingleLabel() = default;

    /// If set, this marker should be made selectable
    ///  and it will be if the selection layer has been set
    bool isSelectable;
    /// If the marker is selectable, this is the unique identifier
    ///  for it.  You should set this ahead of time
    SimpleIdentity selectID;
        
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
    /// If non-zero, this is the texture to use as an icon
    SimpleIdentity iconTexture;
    /// If the texture is set and this is non-zero the size of the image
    Point2f iconSize;
    /// If set, this moves the label if displayed in screen (2D) mode
    Point2d screenOffset;
    /// If set, this overrides the natural size of the label for layout purposes
    Point2d layoutSize;
    /// If non-empty, used to identify a set of labels of which only one should be displayed
    std::string uniqueID;
    /// Set if we're participating in layout
    bool layoutEngine;
    /// Layout importance, if being used
    float layoutImportance;
    /// Layout placement
    int layoutPlacement;
    /// Shape for label to follow
    VectorRing layoutShape;

    // If set, we'll draw an outline to the mask target
    WhirlyKit::SimpleIdentity maskID;
    WhirlyKit::SimpleIdentity maskRenderTargetID;

    /// Some attributes can be overridden per label
    LabelInfoRef infoOverride;

    // Used to build the drawable string on specific platforms
    virtual std::vector<DrawableString *> generateDrawableStrings(
            PlatformThreadInfo *threadInfo,
            const LabelInfo *,
            const FontTextureManagerRef &fontTexManager,
            float &lineHeight,
            ChangeSet &changes) = 0;
};
typedef std::shared_ptr<SingleLabel> SingleLabelRef;
    
#define kWKLabelManager "WKLabelManager"

/** The label manager controls resources for text labels, including construction
    and destruction.  All methods (other than the destructor) are thread safe.
  */
class LabelManager : public SceneManager
{
public:
    LabelManager();
    virtual ~LabelManager() = default;

    /// Add the given set of labels, returning an ID that represents the whole thing
    SimpleIdentity addLabels(PlatformThreadInfo *threadInfo,
                             const std::vector<SingleLabel *> &labels,
                             const LabelInfo &desc,ChangeSet &changes);
    SimpleIdentity addLabels(PlatformThreadInfo *threadInfo,
                             const std::vector<SingleLabelRef> &labels,
                             const LabelInfo &desc,ChangeSet &changes);

    using CancelFunction = std::function<bool(PlatformThreadInfo *)>;
    SimpleIdentity addLabels(PlatformThreadInfo *threadInfo,
                             const std::vector<SingleLabelRef> &labels,
                             const LabelInfo &desc,ChangeSet &changes,
                             const CancelFunction& cancelFn);
    SimpleIdentity addLabels(PlatformThreadInfo *threadInfo,
                             const std::vector<SingleLabel *> &labels,
                             const LabelInfo &desc,ChangeSet &changes,
                             const CancelFunction& cancelFn);

    /// Change visual attributes (just the visibility range)
    void changeLabel(PlatformThreadInfo *threadInfo,
                     SimpleIdentity labelID,
                     const LabelInfo &desc,
                     ChangeSet &changes);
    
    /// Remove the given label(s)
    void removeLabels(PlatformThreadInfo *threadInfo,
                      const SimpleIDSet &labelID,
                      ChangeSet &changes);
    
    /// Enable/disable labels
    void enableLabels(const SimpleIDSet &labelID,bool enable,ChangeSet &changes);
    
protected:
    /// Keep track of labels (or groups of labels) by ID for deletion
    WhirlyKit::LabelSceneRepSet labelReps;
    unsigned int textureAtlasSize;
    SimpleIdentity maskProgID;
};
typedef std::shared_ptr<LabelManager> LabelManagerRef;

}
