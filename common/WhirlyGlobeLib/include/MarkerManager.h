/*  MarkerManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/16/13.
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
 */

#import <math.h>
#import <set>
#import <map>
#import "Identifiable.h"
#import "BasicDrawable.h"
#import "SelectionManager.h"
#import "LayoutManager.h"
#import "Scene.h"
#import "Platform.h"
#import "BaseInfo.h"

namespace WhirlyKit
{
/// Default priority for markers.  At the end, basically
static const int MarkerDrawPriority=10000;

/// Maximum number of triangles we'll stick in a drawable
static const int MaxMarkerDrawableTris=1<<15/3;
    
/// Marker representation.
/// Used internally to track marker resources
class MarkerSceneRep : public Identifiable
{
public:
    MarkerSceneRep();
    ~MarkerSceneRep() = default;

    // Clear the contents out of the scene
    void clearContents(const SelectionManagerRef &selectManager,
                       const LayoutManagerRef &layoutManager,
                       ChangeSet &changes,TimeInterval when);
    
    // Enable/disable marker related features
    void enableContents(const SelectionManagerRef &selectManager,
                        const LayoutManagerRef &layoutManager,
                        bool enable,ChangeSet &changes);

    SimpleIDSet drawIDs;  // Drawables created for this
    SimpleIDSet selectIDs;  // IDs used for selection
    SimpleIDSet screenShapeIDs;  // IDs for screen space objects
    bool useLayout;  // True if we used the layout manager (and thus need to delete)
    float fadeOut;   // Time to fade away for deletion
};
typedef std::set<MarkerSceneRep *,IdentifiableSorter> MarkerSceneRepSet;

// Used to pass marker information between threads
struct MarkerInfo : public BaseInfo
{
    MarkerInfo(bool screenObject);
    MarkerInfo(const Dictionary &,bool screenObject);
    virtual ~MarkerInfo() = default;

    // Convert contents to a string for debugging
    virtual std::string toString() const { return BaseInfo::toString() + " + MarkerInfo..."; }

    RGBAColor color;
    bool screenObject;
    float width,height;
    float layoutImportance;
    int clusterGroup;
    float layoutOffset = 0.0f;
    float layoutSpacing = 20.0f;
    int layoutRepeat = 0;
    bool layoutDebug = false;

    FloatExpressionInfoRef opacityExp;
    ColorExpressionInfoRef colorExp;
    FloatExpressionInfoRef scaleExp;
};
typedef std::shared_ptr<MarkerInfo> MarkerInfoRef;

/** WhirlyKit Marker
 A single marker object to be placed on the globe.  It will show
 up with the given width and height and be selectable if so desired.
 */
class Marker
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    Marker();
    ~Marker() = default;
    
    /// If set, this marker should be made selectable
    ///  and it will be if the selection layer has been set
    bool isSelectable = false;
    /// If the marker is selectable, this is the unique identifier
    ///  for it.  You should set this ahead of time
    WhirlyKit::SimpleIdentity selectID = EmptyIdentity;
    /// The location for the center of the marker.
    WhirlyKit::GeoCoord loc;
    /// Set if this marker is moving
    bool hasMotion = false;
    /// End location if it's moving
    WhirlyKit::GeoCoord endLoc;
    /// Timing for animation, if present
    TimeInterval startTime = 0.0;
    TimeInterval endTime = 0.0;
    /// Color for this marker
    bool colorSet = false;
    RGBAColor color = RGBAColor::white();
    /// The list of textures to use.  If there's just one
    ///  we show that.  If there's more than one, we switch
    ///  between them over the period.
    std::vector<WhirlyKit::SimpleIdentity> texIDs;
    /// If set we'll keep the screen marker upright in screen space
    bool lockRotation = false;
    /// The height in 3-space (remember the globe has radius = 1.0)
    float height = 0.0f;
    /// The width in 3-space (remember the globe has radius = 1.0)
    float width = 0.0f;
    /// Height in screen space to consider for layout
    float layoutHeight = -1.0f;
    /// Width in screen space to consider for layout
    float layoutWidth = -1.0f;
    /// Set if we want a static rotation.  Only matters in screen space
    /// This is rotation clockwise from north in radians
    float rotation = 0.0f;
    /// Offset in points
    WhirlyKit::Point2d offset;
    /// The period over which we'll switch textures
    TimeInterval period = 0.0;
    /// For markers with more than one texture, this is the offset
    ///  we'll use when calculating position within the period.
    TimeInterval timeOffset = 0.0;
    /// Value to use for the layout engine.  Set to MAXFLOAT by
    ///  default, which will always display.
    float layoutImportance = MAXFLOAT;
    /// Shape for label to follow
    VectorRing layoutShape;
    /// Ordering within rendering group
    long orderBy = -1;
    /// Passed through the system as a unique identifier
    std::string uniqueID;
    /// Identifies objects that should be laid out together
    std::string mergeID;

    int layoutPlacement = WhirlyKitLayoutPlacementNone;

    // If set, we'll draw an outline to the mask target
    WhirlyKit::SimpleIdentity maskID = EmptyIdentity;
    WhirlyKit::SimpleIdentity maskRenderTargetID = EmptyIdentity;

    /// A list of vertex attributes to apply to the marker
    SingleVertexAttributeSet vertexAttrs;

    /// Add a texture ID to be displayed
    void addTexID(SimpleIdentity texID);
};
using MarkerVec = std::vector<Marker>;
using MarkerPtrVec = std::vector<Marker *>;

#define kWKMarkerManager "WKMarkerManager"

/** The Marker Manager is used to create and destroy geometry for 2D and 3D markers.
    It's entirely thread safe except for the 
  */
class MarkerManager : public SceneManager
{
public:
    MarkerManager();
    virtual ~MarkerManager();
    
    /// Add an array of markers, returning the identity that corresponds
    // If the markers are selectable, the select ID will be added to each.
    SimpleIdentity addMarkers(const std::vector<Marker> & ,const MarkerInfo &, ChangeSet &);
    SimpleIdentity addMarkers(const std::vector<Marker *> &, const MarkerInfo &, ChangeSet &);

    /// Remove the given set of markers
    void removeMarkers(SimpleIDSet &markerIDs,ChangeSet &changes);
    
    /// Enable/disable markers
    void enableMarkers(SimpleIDSet &markerIDs,bool enable,ChangeSet &changes);
    
    /// Called by the scene once things are set up
    virtual void setScene(Scene *inScene);
    
protected:
    /// Convenience routine to convert the points to model space
    Point3dVector convertGeoPtsToModelSpace(const VectorRing &inPts);

    /// Resources associated with given markers
    MarkerSceneRepSet markerReps;
    /// We route the mask polygons to this program, if there are any
    SimpleIdentity maskProgID;
};
typedef std::shared_ptr<MarkerManager> MarkerManagerRef;

}
