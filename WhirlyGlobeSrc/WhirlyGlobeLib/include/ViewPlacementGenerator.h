/*
 *  ViewPlacementGenerator.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/25/12.
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

#import <Foundation/Foundation.h>
#import <math.h>
#import "WhirlyVector.h"
#import "TextureGroup.h"
#import "GlobeScene.h"
#import "DataLayer.h"
#import "LayerThread.h"
#import "GlobeMath.h"
#import "sqlhelpers.h"
#import "Quadtree.h"
#import "SceneRendererES.h"

namespace WhirlyKit
{

#define kViewPlacementGeneratorShared "SharedViewPlacementGenerator"
  
/** The View Placement Generator moves UIViews around accordingly to locations in geographic coordinates.
    You'll need to both add the UIView here and add it underneath the OpenGL view.
 */
class ViewPlacementGenerator : public WhirlyKit::Generator
{
public:
    ViewPlacementGenerator(const std::string &name);
    virtual ~ViewPlacementGenerator();
    
    /// Used to track a UIView and location we want to put it at
    class ViewInstance
    {
    public:
        ViewInstance() { }
        ViewInstance(UIView *view) : view(view) { }
        ViewInstance(WhirlyKit::GeoCoord loc,UIView *view) : loc(loc), view(view) { offset.x() = view.frame.origin.x;  offset.y() = view.frame.origin.y; }
        ~ViewInstance() { }
        
        /// Sort by UIView
        bool operator < (const ViewInstance &that) const { return view < that.view; }

        /// Where to put the view
        WhirlyKit::GeoCoord loc;
        /// An offset taken from the view origin when it's passed to us
        Point2f offset;
        /// The view we're going to move around
        UIView *view;
        /// Minimum visibility above globe
        float minVis;
        /// Maximum visibility above globe
        float maxVis;
    };
    
    /// Add a view to be tracked.
    /// You should call this from the main thread.
    void addView(GeoCoord loc,UIView *view,float minVis,float maxVis);
    
    /// Remove a view being tracked.
    /// Call this in the main thread
    void removeView(UIView *view);

    /// Rather than generate drawables here, we update our locations
    void generateDrawables(WhirlyKit::RendererFrameInfo *frameInfo,std::vector<DrawableRef> &drawables,std::vector<DrawableRef> &screenDrawables);        
            
    /// Print out stats for debugging
    void dumpStats();
            
protected:
    std::set<ViewInstance> viewInstanceSet;
};
    
}
