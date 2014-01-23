/*
 *  WhirlyKitView.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/9/12.
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

#import "Platform.h"
#import "WhirlyVector.h"
#import "WhirlyKitView.h"
#import "WhirlyGeometry.h"

using namespace Eigen;

namespace WhirlyKit
{

View::View()
{
    fieldOfView = 60.0 / 360.0 * 2 * (float)M_PI;  // 60 degree field of view
    nearPlane = 0.001;
    imagePlaneSize = nearPlane * tanf(fieldOfView / 2.0);
    farPlane = 4.0;
    lastChangedTime = TimeGetCurrent();
    continuousZoom = false;
}
    
View::~View()
{
}

void View::calcFrustumWidth(unsigned int frameWidth,unsigned int frameHeight,Point2d &ll,Point2d &ur,double & near,double &far)
{
	ll.x() = -imagePlaneSize;
	ur.x() = imagePlaneSize;
	double ratio =  ((double)frameHeight / (double)frameWidth);
	ll.y() = -imagePlaneSize * ratio;
	ur.y() = imagePlaneSize * ratio ;
	near = nearPlane;
	far = farPlane;
}

void View::cancelAnimation()
{
}

void View::animate()
{
}

float View::calcZbufferRes()
{
    return 1.0;
}

/// Generate the model view matrix for use by OpenGL.
Eigen::Matrix4d View::calcModelMatrix()
{
    Eigen::Matrix4d ident = ident.Identity();
    return ident;
}

Eigen::Matrix4d View::calcViewMatrix()
{
    Eigen::Matrix4d ident = ident.Identity();
    return ident;
}

Eigen::Matrix4d View::calcFullMatrix()
{
    return calcViewMatrix() * calcModelMatrix();
}

Eigen::Matrix4d View::calcProjectionMatrix(Point2f frameBufferSize,float margin)
{
	float near=0,far=0;
	Point2d frustLL,frustUR;
	frustLL.x() = -imagePlaneSize * (1.0 + margin);
	frustUR.x() = imagePlaneSize * (1.0 + margin);
	double ratio =  ((double)frameBufferSize.y() / (double)frameBufferSize.x());
	frustLL.y() = -imagePlaneSize * ratio * (1.0 + margin);
	frustUR.y() = imagePlaneSize * ratio * (1.0 + margin);
	near = nearPlane;
	far = farPlane;
    
    
    // Borrowed from the "OpenGL ES 2.0 Programming" book
    Eigen::Matrix4d projMat;
    Point3d delta(frustUR.x()-frustLL.x(),frustUR.y()-frustLL.y(),far-near);
    projMat.setIdentity();
    projMat(0,0) = 2.0f * near / delta.x();
    projMat(1,0) = projMat(2,0) = projMat(3,0) = 0.0f;
    
    projMat(1,1) = 2.0f * near / delta.y();
    projMat(0,1) = projMat(2,1) = projMat(3,1) = 0.0f;
    
    projMat(0,2) = (frustUR.x()+frustLL.x()) / delta.x();
    projMat(1,2) = (frustUR.y()+frustLL.y()) / delta.y();
    projMat(2,2) = -(near + far ) / delta.z();
    projMat(3,2) = -1.0f;
    
    projMat(2,3) = -2.0f * near * far / delta.z();
    projMat(0,3) = projMat(1,3) = projMat(3,3) = 0.0f;
    
    return projMat;
}

double View::heightAboveSurface()
{
    return 0.0;
}

Point3d View::pointUnproject(Point2f screenPt,unsigned int frameWidth,unsigned int frameHeight,bool clip)
{
	Point2d ll,ur;
	double near,far;
	calcFrustumWidth(frameWidth,frameHeight,ll,ur,near,far);
	
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

//- (WhirlyKit::Ray3f)displaySpaceRayFromScreenPt:(WhirlyKit::Point2f)screenPt width:(float)frameWidth height:(float)frameHeight
//{
//    // Here's where that screen point is in display space
//    Point3f dispPt = [self pointUnproject:screenPt width:frameWidth height:frameHeight clip:false];
//
//    Eigen::Matrix4f modelMat = [self calcModelMatrix].inverse();
//	
//	Vector4f newUp = modelMat * Vector4f(0,0,1,1);
//	Vector3f eyePt(newUp.x(),newUp.y(),newUp.z());
//        
//    return Ray3f(eyePt,(dispPt-eyePt).normalized());
//}

/// Add a watcher delegate
void View::addWatcher(ViewWatcher *watcher)
{
    watchers.insert(watcher);
}

/// Remove the given watcher delegate
void View::removeWatcher(ViewWatcher *watcher)
{
    watchers.erase(watcher);
}

void View::runViewUpdates()
{
    for (ViewWatcherSet::iterator it = watchers.begin();
         it != watchers.end(); ++it)
        (*it)->viewUpdated(this);
}

}
