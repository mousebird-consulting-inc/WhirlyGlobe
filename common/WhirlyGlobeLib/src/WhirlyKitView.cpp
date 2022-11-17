/*  WhirlyKitView.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/9/12.
 *  Copyright 2011-2022 mousebird consulting
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

#import "Platform.h"
#import "WhirlyKitLog.h"
#import "WhirlyVector.h"
#import "WhirlyKitView.h"
#import "WhirlyGeometry.h"
#import "FlatMath.h"
#import "SceneRenderer.h"

using namespace Eigen;

namespace WhirlyKit
{

View::View()
{
    updateParams();
}

View::View(const View &that) :
    fieldOfView(that.fieldOfView),
    imagePlaneSize(that.imagePlaneSize),
    nearPlane(that.nearPlane),
    farPlane(that.farPlane),
    centerOffset(that.centerOffset),
    lastChangedTime(that.lastChangedTime),
    coordAdapter(that.coordAdapter),
    continuousZoom(that.continuousZoom)
{
}

void View::setFieldOfView(float fov)
{
    fieldOfView = fov;
    updateParams();
}

void View::setNearPlane(float near)
{
    nearPlane = near;
    updateParams();
}

void View::setFarPlane(float far)
{
    farPlane = far;
    updateParams();
}

void View::setPlanes(float near, float far)
{
    nearPlane = near;
    farPlane = far;
    updateParams();
}

void View::updateParams()
{
    imagePlaneSize = nearPlane * std::tan(fieldOfView / 2.0);
    lastChangedTime = TimeGetCurrent();
}

void View::calcFrustumWidth(unsigned int frameWidth,unsigned int frameHeight,Point2d &ll,Point2d &ur,double & near,double &far)
{
    if (frameWidth == 0)
    {
        return;
    }
	ll.x() = -imagePlaneSize;
	ur.x() = imagePlaneSize;
	const double ratio =  ((double)frameHeight / (double)frameWidth);
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
    return 1.0f;
}

/// Generate the model view matrix for use by OpenGL.
Eigen::Matrix4d View::calcModelMatrix() const
{
    return Eigen::Matrix4d::Identity();
}

Eigen::Matrix4d View::calcViewMatrix() const
{
    return Eigen::Matrix4d::Identity();
}

Eigen::Matrix4d View::calcFullMatrix() const
{
    return calcViewMatrix() * calcModelMatrix();
}

Eigen::Matrix4d View::calcProjectionMatrix(Point2f frameBufferSize,float margin) const
{
    const double ratio = (double)frameBufferSize.y() / (double)frameBufferSize.x();
    const double size = imagePlaneSize * (1.0 + margin);
    const Point2d frustLL = Point2d(-1, -ratio) * size;
    const Point2d frustUR = Point2d(1, ratio) * size;
    const Point3d delta(frustUR.x()-frustLL.x(),frustUR.y()-frustLL.y(),farPlane-nearPlane);

    // Borrowed from the "OpenGL ES 2.0 Programming" book
    Eigen::Matrix4d projMat = Eigen::Matrix4d::Identity();
    projMat(0,0) = 2.0f * nearPlane / delta.x();
    projMat(1,0) = projMat(2,0) = projMat(3,0) = 0.0f;
    
    projMat(1,1) = 2.0f * nearPlane / delta.y();
    projMat(0,1) = projMat(2,1) = projMat(3,1) = 0.0f;
    
    projMat(0,2) = (frustUR.x()+frustLL.x()) / delta.x();
    projMat(1,2) = (frustUR.y()+frustLL.y()) / delta.y();
    projMat(2,2) = -(nearPlane + farPlane) / delta.z();
    projMat(3,2) = -1.0f;

    projMat(2,3) = -2.0f * nearPlane * farPlane / delta.z();
    projMat(0,3) = projMat(1,3) = projMat(3,3) = 0.0f;
    
    return projMat;
}

void View::getOffsetMatrices(Matrix4dVector &matrices,
                             const WhirlyKit::Point2f &frameBufferSize,float bufferX) const
{
    matrices.emplace_back(Eigen::Matrix4d::Identity());
}

WhirlyKit::Point2f View::unwrapCoordinate(const WhirlyKit::Point2f &pt) const
{
    return pt;
}

double View::heightAboveSurface() const
{
    return 0.0;
}

Eigen::Vector3d View::eyePos() const
{
    return { 0.0, 0.0, 0.0 };
}

Point3d View::pointUnproject(Point2f screenPt,unsigned int frameWidth,unsigned int frameHeight,bool clip)
{
	Point2d ll,ur;
	double near,far;
	calcFrustumWidth(frameWidth,frameHeight,ll,ur,near,far);
	
	// Calculate a parametric value and flip the y/v
	double u = screenPt.x() / (float)frameWidth;
    if (clip)
    {
        u = std::max(0.0,u);	u = std::min(1.0,u);
    }
	double v = screenPt.y() / (float)frameHeight;
    if (clip)
    {
        v = std::max(0.0,v);	v = std::min(1.0,v);
    }
	v = 1.0 - v;
	
	// Now come up with a point in 3 space between ll and ur
	const Point2d mid(u * (ur.x()-ll.x()) + ll.x(), v * (ur.y()-ll.y()) + ll.y());
	return { mid.x(), mid.y(), -near };
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


double View::currentMapScale(const WhirlyKit::Point2f &frameSize) const
{
    double scale = (2 * heightAboveSurface() *  tan(fieldOfView/2.0) * EarthRadius) / (frameSize.x() * 0.00096) ;
    return scale;
}

double View::heightForMapScale(double scale,const WhirlyKit::Point2f &frameSize) const
{
    const double height = (scale * frameSize.x() * 0.00096) / (2 * tan(fieldOfView/2.0) * EarthRadius);
    return height;
}

double View::currentMapZoom(const WhirlyKit::Point2f &frameSize,double latitude) const
{
  const double mapWidthInMeters = (2 * heightAboveSurface() *  tan(fieldOfView/2.0) * EarthRadius);
  const double metersPerPixel = mapWidthInMeters/frameSize.x();
  return log(EarthRadius * RadToDeg(cos(latitude))/ metersPerPixel)/log(2.0) - 8;
}

Point2d View::screenSizeInDisplayCoords(const Point2f &frameSize)
{
    Point2d screenSize(0,0);
    if (frameSize.x() == 0.0 || frameSize.y() == 0.0)
        return screenSize;
    
    screenSize.x() = tan(fieldOfView/2.0) * heightAboveSurface() * 2.0;
    screenSize.y() = screenSize.x() / frameSize.x() * frameSize.y();
    
    return screenSize;
}

/// Add a watcher delegate
void View::addWatcher(const ViewWatcherRef &watcher)
{
    std::lock_guard<std::mutex> guardLock(watcherLock);
    removeWatcherLocked(watcher);
    watchers.push_back(watcher);
}

/// Remove the given watcher delegate
void View::removeWatcher(const ViewWatcherRef &watcher)
{
    std::lock_guard<std::mutex> guardLock(watcherLock);
    removeWatcherLocked(watcher);
}

void View::removeWatcherLocked(const ViewWatcherRef &watcher)
{
    // Remove items from the watcher list if they match the given reference or reference dead watchers.
    const ViewWatcher *ptr = watcher.get();
    const auto pred = [&](auto &weak){
        auto ref = weak.lock();
        return !ref || ref.get() == ptr;
    };
    watchers.erase(std::remove_if(watchers.begin(), watchers.end(), pred), watchers.end());
}

void View::runViewUpdates()
{
    // Make a copy so we don't block everyone while running updates
    std::vector<ViewWatcherWeakRef> watchersToRun;
    {
        std::lock_guard<std::mutex> guardLock(watcherLock);
        watchersToRun = watchers;
    }
    for (auto &weakRef : watchersToRun)
    {
        if (auto watcher = weakRef.lock())
        {
            watcher->viewUpdated(this);
        }
    }
}

ViewState::ViewState(WhirlyKit::View *view,SceneRenderer *renderer) :
    near(0),
    far(0)
{
    modelMatrix = view->calcModelMatrix();
    invModelMatrix = modelMatrix.inverse();
    
    Matrix4dVector offMatrices;
    Point2f frameSize = renderer->getFramebufferSize();
    view->getOffsetMatrices(offMatrices, frameSize, 0.0);
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
    
    // Calculate a parametric value and flip the y/v
    double u = screenPt.x() / frameWidth;
    if (clip)
    {
        u = std::max(0.0,u);    u = std::min(1.0,u);
    }
    double v = screenPt.y() / frameHeight;
    if (clip)
    {
        v = std::max(0.0,v);    v = std::min(1.0,v);
    }
    v = 1.0 - v;
    
    // Now come up with a point in 3 space between ll and ur
    const Point2d mid(u * (ur.x()-ll.x()) + ll.x(), v * (ur.y()-ll.y()) + ll.y());
    return { mid.x(),mid.y(),-near };
}

Point2f ViewState::pointOnScreenFromDisplay(const Point3d &worldLoc,const Eigen::Matrix4d *transform,const Point2f &frameSize)
{
    // Run the model point through the model transform (presumably what they passed in)
    const Eigen::Matrix4d modelMat = *transform;
    const Vector4d screenPt = modelMat * Vector4d(worldLoc.x(),worldLoc.y(),worldLoc.z(),1.0);
    
    // Intersection with near gives us the same plane as the screen
    Vector3d ray = Point3d(screenPt.x(), screenPt.y(), screenPt.z()) / screenPt.w();
    ray *= -nearPlane/ray.z();
    
    // Now we need to scale that to the frame
    if (ll.x() == ur.x())
    {
        calcFrustumWidth((int)frameSize.x(),(int)frameSize.y());
    }

    const auto u = (float)((ray.x() - ll.x()) / (ur.x() - ll.x()));
    const auto v = (float)(1.0 - (ray.y() - ll.y()) / (ur.y() - ll.y()));

    return (ray.z() < 0.0) ? Point2f{ u * frameSize.x(), v * frameSize.y() } :
                             Point2f{ -100000.0f, -100000.0f };
}

bool ViewState::isSameAs(const ViewState *other) const
{
    if (fieldOfView != other->fieldOfView || imagePlaneSize != other->imagePlaneSize ||
        nearPlane != other->nearPlane || farPlane != other->farPlane)
        return false;
    
    // Matrix comparison
    const double *floatsA = fullMatrices[0].data();
    const double *floatsB = other->fullMatrices[0].data();
    for (unsigned int ii=0;ii<16;ii++)
        if (floatsA[ii] != floatsB[ii])
            return false;

    return true;
}

void ViewState::log()
{
    wkLogLevel(Verbose,"--- ViewState ---");
    wkLogLevel(Verbose,"eyeVec = (%f,%f,%f), eyeVecModel = (%f,%f,%f)",eyeVec.x(),eyeVec.y(),eyeVec.z(),eyeVecModel.x(),eyeVecModel.y(),eyeVecModel.z());
    for (auto &fullMatrice : fullMatrices)
    {
        std::stringstream strStrm;
        for (unsigned int ii=0;ii<16;ii++)
            strStrm << fullMatrice.data()[ii];
        wkLogLevel(Verbose,"fullMatrix = %@",strStrm.str().c_str());
    }
    wkLogLevel(Verbose,"---     ---   ---");
}
    
}
