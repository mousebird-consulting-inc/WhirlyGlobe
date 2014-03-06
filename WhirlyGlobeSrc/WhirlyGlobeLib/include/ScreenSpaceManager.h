/*
 *  ScreenSpaceManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/21/14.
 *  Copyright 2011-2014 mousebird consulting. All rights reserved.
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
#import "LayoutManager.h"
#import "Scene.h"

namespace WhirlyKit
{
    
/// Used to track the visual representation of screen space objects
class ScreenSpaceRep : public Identifiable
{
public:
    ScreenSpaceRep();
    ~ScreenSpaceRep() { }

    // Clear the contents out of the scene
    void clearContents(ChangeSet &changes);
    
    // Enable/disable marker related features
    void enableContents(bool enable,ChangeSet &changes);
    
    SimpleIDSet drawIDs;
    
    float fade;
};
    
typedef std::set<ScreenSpaceRep *,IdentifiableSorter> ScreenSpaceRepSet;

#define kWKScreenSpaceManager "WKScreenSpaceManager"

/** The Screen Space manager handles drawables that live in 2D space.
    This replaces the screen space generator that used to run on the main thread.
  */
class ScreenSpaceManager : public SceneManager
{
public:
    ScreenSpaceManager();
    virtual ~ScreenSpaceManager();
    
    /// A simple geometric representation used in shapes
    class SimpleGeometry
    {
    public:
        SimpleGeometry();
        SimpleGeometry(SimpleIdentity texID,SimpleIdentity programID,RGBAColor color,const std::vector<Point2f> &coords,const std::vector<TexCoord> &texCoords);
        
        SimpleIdentity texID;
        SimpleIdentity programID;
        RGBAColor color;
        std::vector<Point2f> coords;
        std::vector<TexCoord> texCoords;
    };
    
    /** Simple convex shape to be drawn on the screen.
     It has a texture and a list of vertices as well as
     the usual minVis/maxVis values and draw priority.
     */
    class ConvexShape : public Identifiable
    {
    public:
        ConvexShape();
        
        /// Center location
        Point3f worldLoc;
        /// If true we'll use the rotation.  If not, we won't.
        bool useRotation;
        /// Rotation clockwise from north
        float rotation;
        /// If we're fading in or out, these are used
        NSTimeInterval fadeUp,fadeDown;
        /// Sort by draw priority
        int drawPriority;
        /// Visual range
        float minVis,maxVis;
        /// 2D offset to be applied (probably from the layout engine)
        Point2f offset;
        /// false if we're not to draw this one
        bool enable;
        
        /// List of geometry we'll transform to the destination
        std::vector<SimpleGeometry> geom;
    };
    
    /// Add a set of shapes identified by a single ID
    SimpleIdentity addShapes(const std::vector<ConvexShape> &shapes,ChangeSet &changes);
    
    /// Enable/disable shapes
    void enableShapes(const SimpleIDSet &shapeIDs,bool enable,ChangeSet &changes);
    
    /// Remove a set of shapes
    void removeShapes(const SimpleIDSet &shapeIDs,ChangeSet &changes);
    
protected:
    pthread_mutex_t shapeLock;
    ScreenSpaceRepSet shapeReps;
};
    
}
