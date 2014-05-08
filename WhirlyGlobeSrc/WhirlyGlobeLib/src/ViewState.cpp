/*
 *  LayerViewWatcher.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/28/12.
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

#import "ViewState.h"
// Note: Porting
//#import "LayerThread.h"
#import "SceneRendererES.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit
{
    
ViewState::ViewState(WhirlyKit::View *view,SceneRendererES *renderer)
{
    modelMatrix = view->calcModelMatrix();
    invModelMatrix = modelMatrix.inverse();
    
    std::vector<Eigen::Matrix4d> offMatrices;
    Point2f frameSize = renderer->getFramebufferSize();
    view->getOffsetMatrices(offMatrices, frameSize);
    viewMatrices.resize(offMatrices.size());
    invViewMatrices.resize(offMatrices.size());
    fullMatrices.resize(offMatrices.size());
    invFullMatrices.resize(offMatrices.size());
    fullNormalMatrices.resize(offMatrices.size());

    projMatrix = view->calcProjectionMatrix(renderer->getFramebufferSize(),0.0);
    invProjMatrix = projMatrix.inverse();
    Eigen::Matrix4d baseViewMatrix = view->calcViewMatrix();
    for (unsigned int ii=0;ii<offMatrices.size();ii++)
    {
        viewMatrices[ii] = baseViewMatrix * offMatrices[ii];
        invViewMatrices[ii] = viewMatrices[ii].inverse();
        fullMatrices[ii] = viewMatrices[ii] * modelMatrix;
        invFullMatrices[ii] = fullMatrices[ii].inverse();
        fullNormalMatrices[ii] = fullMatrices[ii].inverse().transpose();
    }
    
    fieldOfView = view->fieldOfView;
    imagePlaneSize = view->imagePlaneSize;
    nearPlane = view->nearPlane;
    farPlane = view->farPlane;
    
    // Need the eye point for backface checking
    Vector4d eyeVec4 = invFullMatrices[0] * Vector4d(0,0,1,0);
    eyeVec = Vector3d(eyeVec4.x(),eyeVec4.y(),eyeVec4.z());
    // Also a version for the model matrix (e.g. just location, not direction)
    eyeVec4 = invModelMatrix * Vector4d(0,0,1,0);
    eyeVecModel = Vector3d(eyeVec4.x(),eyeVec4.y(),eyeVec4.z());
    // And calculate where the eye actually is
    Vector4d eyePos4 = invFullMatrices[0] * Vector4d(0,0,0,1);
    eyePos = Vector3d(eyePos4.x(),eyePos4.y(),eyePos4.z());
    
    ll.x() = ur.x() = 0.0;
    
    coordAdapter = view->coordAdapter;
}
    
ViewState::~ViewState()
{
}
    
void ViewState::calcFrustumWidth(unsigned int frameWidth,unsigned int frameHeight)
{
	ll.x() = -imagePlaneSize;
	ur.x() = imagePlaneSize;
	float ratio =  ((float)frameHeight / (float)frameWidth);
	ll.y() = -imagePlaneSize * ratio;
	ur.y() = imagePlaneSize * ratio ;
	near = nearPlane;
	far = farPlane;
}

Point3d ViewState::pointUnproject(Point2d screenPt,unsigned int frameWidth,unsigned int frameHeight,bool clip)
{
    if (ll.x() == ur.x())
        calcFrustumWidth(frameWidth,frameHeight);
	
	// Calculate a parameteric value and flip the y/v
	double u = screenPt.x() / frameWidth;
    if (clip)
    {
        u = std::max(0.0,u);	u = std::min(1.0,u);
    }
	double v = screenPt.y() / frameHeight;
    if (clip)
    {
        v = std::max(0.0,v);	v = std::min(1.0,v);
    }
	v = 1.0 - v;
	
	// Now come up with a point in 3 space between ll and ur
	Point2d mid(u * (ur.x()-ll.x()) + ll.x(), v * (ur.y()-ll.y()) + ll.y());
	return Point3d(mid.x(),mid.y(),-near);
}

Point2f ViewState::pointOnScreenFromDisplay(const Point3d &worldLoc,const Eigen::Matrix4d *transform,const Point2f &frameSize)
{
    // Run the model point through the model transform (presumably what they passed in)
    Eigen::Matrix4d modelMat = *transform;
    Vector4d screenPt = modelMat * Vector4d(worldLoc.x(),worldLoc.y(),worldLoc.z(),1.0);
    
    // Intersection with near gives us the same plane as the screen
    Vector3d ray;
    ray.x() = screenPt.x() / screenPt.w();  ray.y() = screenPt.y() / screenPt.w();  ray.z() = screenPt.z() / screenPt.w();
    ray *= -nearPlane/ray.z();
    
    // Now we need to scale that to the frame
    if (ll.x() == ur.x())
        calcFrustumWidth(frameSize.x(),frameSize.y());
    double u = (ray.x() - ll.x()) / (ur.x() - ll.x());
    double v = (ray.y() - ll.y()) / (ur.y() - ll.y());
    v = 1.0 - v;
    
    Point2f retPt;
    if (ray.z() < 0.0)
    {
       retPt.x() = u * frameSize.x();
       retPt.y() = v * frameSize.y();
    } else
        retPt = Point2f(-100000, -100000);
    
    return retPt;
}

bool ViewState::isSameAs(WhirlyKit::ViewState *other)
{
    if (fieldOfView != other->fieldOfView || imagePlaneSize != other->imagePlaneSize ||
        nearPlane != other->nearPlane || farPlane != other->farPlane)
        return false;
    
    // Matrix comparison
    double *floatsA = fullMatrices[0].data();
    double *floatsB = other->fullMatrices[0].data();
    for (unsigned int ii=0;ii<16;ii++)
        if (floatsA[ii] != floatsB[ii])
            return false;

    return true;
}

void ViewState::log()
{
    // Note: Porting
//    NSLog(@"--- ViewState ---");
//    NSLog(@"eyeVec = (%f,%f,%f), eyeVecModel = (%f,%f,%f)",eyeVec.x(),eyeVec.y(),eyeVec.z(),eyeVecModel.x(),eyeVecModel.y(),eyeVecModel.z());
//    NSMutableString *matStr = [NSMutableString string];
//    for (unsigned int ii=0;ii<16;ii++)
//        [matStr appendFormat:@" %f",fullMatrix.data()[ii]];
//    NSLog(@"fullMatrix = %@",matStr);
//    NSLog(@"---     ---   ---");
}

}
