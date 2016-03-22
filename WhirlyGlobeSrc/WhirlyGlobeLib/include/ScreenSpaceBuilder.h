/*
 *  ScreenSpaceBuild.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/21/14.
 *  Copyright 2011-2016 mousebird consulting.
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
#import "ScreenSpaceDrawable.h"
#import "Scene.h"

namespace WhirlyKit
{
    
class ScreenSpaceObject;
class ScreenSpaceObjectLocation;
    
/** Screen space objects are used for both labels and markers.  This builder
    helps construct the drawables needed to represent them.
  */
class ScreenSpaceBuilder
{
public:
    ScreenSpaceBuilder(CoordSystemDisplayAdapter *coordAdapter,float scale,float centerDist=10e2);
    virtual ~ScreenSpaceBuilder();
    
    // State information we're keeping around.
    // Defaults to something resonable
    class DrawableState
    {
    public:
        DrawableState();
        
        // Comparison operator for set
        bool operator < (const DrawableState &that) const;
        
        std::vector<SimpleIdentity> texIDs;
        double period;
        SimpleIdentity progID;
        TimeInterval fadeUp,fadeDown;
        bool enable;
        TimeInterval startEnable,endEnable;
        int drawPriority;
        float minVis,maxVis;
        bool motion;
        bool rotation;
        bool keepUpright;
        SingleVertexAttributeInfoSet vertexAttrs;
    };
    
    /// Draw priorities can mix and match with other objects, but we probably don't want that
    void setDrawPriorityOffset(int drawPriorityOffset);
    
    /// Set the active texture ID
    void setTexID(SimpleIdentity texID);
    /// Set the active texture IDs
    void setTexIDs(const std::vector<SimpleIdentity> &texIDs,double period);
    /// Set the active program ID
    void setProgramID(SimpleIdentity progID);
    /// Set the fade in/out
    void setFade(TimeInterval fadeUp,TimeInterval fadeDown);
    /// Set the draw priority
    void setDrawPriority(int drawPriority);
    /// Set the visibility range
    void setVisibility(float minVis,float maxVis);
    /// Set the start enable
    void setEnable(bool enable);
    /// Set the enable time range
    void setEnableRange(TimeInterval inStartEnable,TimeInterval inEndEnable);

    /// Add a single rectangle with no rotation
    void addRectangle(const Point3d &worldLoc,const Point2d *coords,const TexCoord *texCoords,const RGBAColor &color);
    /// Add a single rectangle with rotation, possibly keeping upright
    void addRectangle(const Point3d &worldLoc,double rotation,bool keepUpright,const Point2d *coord,const TexCoord *texCoords,const RGBAColor &color);

    /// Add a whole bunch of predefined Scene Objects
    void addScreenObjects(std::vector<ScreenSpaceObject> &screenObjects);
    
    /// Add a single screen space object
    void addScreenObject(const ScreenSpaceObject &screenObject);
    
    /// Return the drawables constructed.  Caller responsible for deletion.
    void buildDrawables(std::vector<ScreenSpaceDrawable *> &draws);
    
    /// Build drawables and add them to the change list
    void flushChanges(ChangeSet &changes,SimpleIDSet &drawIDs);
    
protected:
    // Wrapper used to track
    class DrawableWrap
    {
    public:
        DrawableWrap();
        DrawableWrap(const DrawableState &state);
        ~DrawableWrap();
        
        // Comparison operator for set
        bool operator < (const DrawableWrap &that) const;
        
        void addVertex(CoordSystemDisplayAdapter *coordAdapter,float scale,const Point3f &worldLoc,float rot,const Point2f &vert,const TexCoord &texCoord,const RGBAColor &color,const SingleVertexAttributeSet *vertAttrs);
        void addVertex(CoordSystemDisplayAdapter *coordAdapter,float scale,const Point3f &worldLoc,const Point3f &dir,float rot,const Point2f &vert,const TexCoord &texCoord,const RGBAColor &color,const SingleVertexAttributeSet *vertAttrs);
        void addTri(int v0,int v1,int v2);
        
        Point3d center;
        DrawableState state;
        ScreenSpaceDrawable *draw;
        
    protected:
        Point3f calcRotationVec(CoordSystemDisplayAdapter *coordAdapter,const Point3f &worldLoc,float rot);        
    };

    // Comparitor for drawable wrapper set
    typedef struct
    {
        bool operator()(const DrawableWrap *a,const DrawableWrap *b)
        {
            return *a < *b;
        }
    } DrawableWrapComparator;
    
    typedef std::set<DrawableWrap *,DrawableWrapComparator> DrawableWrapSet;
    
    DrawableWrap *findOrAddDrawWrap(const DrawableState &state,int numVerts,int numTri,const Point3d &center);
    
    float centerDist;
    float scale;
    int drawPriorityOffset;
    CoordSystemDisplayAdapter *coordAdapter;
    DrawableState curState;
    DrawableWrapSet drawables;
    std::vector<DrawableWrap *> fullDrawables;
};
    
/** Keeps track of the basic information about a screen space object.
    These are passed around by the
 */
class ScreenSpaceObject : public Identifiable
{
public:
    friend class LayoutManager;
    friend class ScreenSpaceBuilder;
    
    ScreenSpaceObject();
    ScreenSpaceObject(SimpleIdentity theId);
    virtual ~ScreenSpaceObject();
    
    /// Represents a simple set of convex geometry
    class ConvexGeometry
    {
    public:
        ConvexGeometry();
        
        /// Texture ID used for just this object
        std::vector<SimpleIdentity> texIDs;
        /// Program ID used to render this geometry
        SimpleIdentity progID;
        /// Color for the geometry
        RGBAColor color;
        /// Vertex attributes applied to this piece of geometry
        SingleVertexAttributeSet vertexAttrs;
        
        Point2dVector coords;
        std::vector<TexCoord> texCoords;
    };
    
    /// Center of the object in world coordinates
    void setWorldLoc(const Point3d &worldLoc);
    Point3d getWorldLoc();
    Point3d getEndWorldLoc();
    TimeInterval getStartTime(),getEndTime();
    
    /// Set up the end location and timing
    void setMovingLoc(const Point3d &worldLoc,TimeInterval startTime,TimeInterval endTime);
    
    void setEnable(bool enable);
    void setEnableTime(TimeInterval startEnable,TimeInterval endEnable);
    void setVisibility(float minVis,float maxVis);
    void setDrawPriority(int drawPriority);
    void setKeepUpright(bool keepUpright);
    void setRotation(double rotation);
    void setFade(TimeInterval fadeUp,TimeInterval fadeDown);
    void setOffset(const Point2d &offset);
    void setPeriod(TimeInterval period);
    
    void addGeometry(const ConvexGeometry &geom);
    
protected:
    bool enable;
    TimeInterval startEnable,endEnable;
    Point3d worldLoc,endWorldLoc;
    TimeInterval startTime,endTime;
    Point2d offset;
    double rotation;
    bool keepUpright;
    ScreenSpaceBuilder::DrawableState state;
    std::vector<ConvexGeometry> geometry;
};
    
/** We use the screen space object location to communicate where
    a screen space object is on the screen.
  */
class ScreenSpaceObjectLocation
{
public:
    ScreenSpaceObjectLocation();

    // ID for selector
    SimpleIdentity shapeID;
    // Location of object in display space
    Point3d dispLoc;
    // Offset on the screen (presumably if it's been moved around during layout)
    Point2d offset;
    // Size of the object in screen space
    Point2dVector pts;
    // Bounding box, for quick testing
    Mbr mbr;
};
    
}
