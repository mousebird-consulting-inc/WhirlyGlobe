/*
 *  IntersectionManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/29/16.
 *  Copyright 2011-2016 mousebird consulting. All rights reserved.
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
#import "WhirlyGeometry.h"
#import "WhirlyKitView.h"
#import "MaplyView.h"
#import "Scene.h"
#import "ScreenSpaceBuilder.h"

namespace WhirlyKit
{
    
#define kWKIntersectionManager "WKIntersectionManager"
    
/** The Intersection Manager keeps track of objects that can provide
    intersection feedback.  We use these for gestures moving around data
    sets in 3D.  It differs from the SelectionManager in that it's
    just used for 3D motion.
  */
class IntersectionManager : public SceneManager
{
public:
    IntersectionManager(Scene *scene);
    ~IntersectionManager();

    /** A base class for intersectable sets of objects.
        Fill in the methods and return the closest valid intersection.
      */
    class Intersectable
    {
    public:
        virtual ~Intersectable();

        // Ray is in display coordinates
        virtual bool findClosestIntersection(SceneRendererES *renderer,View *theView,const Point2f &frameSize,const Point2f &touchPt,const Point3d &org,const Point3d &dir,Point3d &iPt,double &dist) = 0;
    };
    
    /// Add an intersectable object
    void addIntersectable(Intersectable *intersect);
    
    /// Remove an intersectable object
    void removeIntersectable(Intersectable *intersect);

    /// Look for the nearest intersection and return the point (in display coordinates)
    bool findIntersection(SceneRendererES *renderer,View *theView,const Point2f &frameSize,const Point2f &touchPt,Point3d &iPt,double &dist);

protected:
    pthread_mutex_t mutex;
    Scene *scene;
    std::set<Intersectable *> intersectables;
};

}
