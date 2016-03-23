/*
 *  ViewState.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/28/12.
 *  Copyright 2011-2016 mousebird consulting
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

#import "WhirlyKitView.h"

namespace WhirlyKit
{
//class ViewStateFactory;
class ViewState;
class SceneRendererES;
    
/// Generate a view state of the appropriate type
class ViewStateFactory
{
public:
    ViewStateFactory() { }
    virtual ~ViewStateFactory() { }
    virtual ViewState *makeViewState(View *,SceneRendererES *renderer) = 0;
};

/** Representation of the view state.  This is the base
 class for specific view state info for the various view
 types.
 */
class ViewState
{
public:
    ViewState() : near(0), far(0) { }
    ViewState(View *view,SceneRendererES *renderer);
    virtual ~ViewState();
    
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    /// Calculate the viewing frustum (which is also the image plane)
    /// Need the framebuffer size in pixels as input
    /// This will cache the values in the view state for later use
    void calcFrustumWidth(unsigned int frameWidth,unsigned int frameHeight);
    
    /// From a screen point calculate the corresponding point in 3-space
    Point3d pointUnproject(Point2d screenPt,unsigned int frameWidth,unsigned int frameHeight,bool clip);
    
    /// From a world location (3D), figure out the projection to the screen
    ///  Returns a point within the frame
    Point2f pointOnScreenFromDisplay(const Point3d &worldLoc,const Eigen::Matrix4d *transform,const Point2f &frameSize);
    
    /// Compare this view state to the other one.  Returns true if they're identical.
    bool isSameAs(WhirlyKit::ViewState *other);
    
    /// Return true if the view state has been set to something
    bool isValid() { return near != far; }
    
    /// Dump out info about the view state
    void log();
    
    Eigen::Matrix4d modelMatrix,projMatrix;
    std::vector<Eigen::Matrix4d> viewMatrices,invViewMatrices,fullMatrices,fullNormalMatrices,invFullMatrices;
    Eigen::Matrix4d invModelMatrix,invProjMatrix;
    double fieldOfView;
    double imagePlaneSize;
    double nearPlane;
    double farPlane;
    Point3d eyeVec;
    Point3d eyeVecModel;
    Point2d ll,ur;
    double near,far;
    CoordSystemDisplayAdapter *coordAdapter;
    
    /// Calculate where the eye is in model coordinates
    Point3d eyePos;
};

}

