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
#import "ScreenSpaceDrawable.h"
#import "Scene.h"

namespace WhirlyKit
{
    
/** Screen space objects are used for both labels and markers.  This builder
    helps construct the drawables needed to represent them.
  */
class ScreenSpaceBuilder
{
public:
    ScreenSpaceBuilder();
    virtual ~ScreenSpaceBuilder();
    
    /// Set the active texture ID
    void setTexID(SimpleIdentity texID);
    /// Set the active program ID
    void setProgramID(SimpleIdentity progID);
    /// Set the fade in/out
    void setFade(NSTimeInterval fadeUp,NSTimeInterval fadeDown);
    /// Set the draw priority
    void setDrawPriority(int drawPriority);
    /// Set the visibility range
    void setVisibility(float minVis,float maxVis);

    /// Add a single rectangle with no rotation
    void addRectangle(const Point3d &worldLoc,const Point3d *coords,const TexCoord *texCoords);
    /// Add a single rectangle with rotation, possibly keeping upright
    void addRectangle(const Point3d &worldLoc,double rotation,bool keepUpright,const Point3d *coord,const TexCoord *texCoords);

    // Return the drawables constructed.  Caller responsible for deletion.
    void buildDrawables(std::vector<ScreenSpaceDrawable *> &draws);
    
protected:

    // State information we're keeping around.
    // Defaults to something resonable
    class DrawableState
    {
    public:
        DrawableState();
        
        // Comparison operator for set
        bool operator < (const DrawableState &that) const;
        
        SimpleIdentity texID;
        SimpleIdentity progID;
        NSTimeInterval fadeUp,fadeDown;
        int drawPriority;
        float minVis,maxVis;
    };

    // Wrapper used to track
    class DrawableWrap
    {
    public:
        DrawableWrap();
        
        void addVertex(const Point3f &worldLoc,float rot,const Point2f &vert,const TexCoord &texCoord,const RGBAColor &color);
        void addTri(int v0,int v1,int v2);
        
        DrawableState state;
        ScreenSpaceDrawable *draw;
    };
    typedef std::set<DrawableWrap *> DrawableWrapSet;
    
    DrawableState curState;
    DrawableWrapSet drawables;
};
    
}
