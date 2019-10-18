/*
 *  WhirlyKitView.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/9/12.
 *  Copyright 2011-2019 mousebird consulting
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
    fieldOfView = 60.0 / 360.0 * 2 * (float)M_PI;  // 60 degree field of view
    nearPlane = 0.001;
    imagePlaneSize = nearPlane * tanf(fieldOfView / 2.0);
    farPlane = 10.0;
    centerOffset = Point2d(0.0,0.0);
    lastChangedTime = TimeGetCurrent();
    continuousZoom = false;
}
    
View::View(const View &that)
    : fieldOfView(that.fieldOfView), nearPlane(that.nearPlane), imagePlaneSize(that.imagePlaneSize),
    farPlane(that.farPlane), lastChangedTime(that.lastChangedTime), continuousZoom(that.continuousZoom),
    coordAdapter(that.coordAdapter)
{
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

void View::getOffsetMatrices(std::vector<Eigen::Matrix4d> &offsetMatrices,const WhirlyKit::Point2f &frameBufferSize,float bufferX)
{
    Eigen::Matrix4d ident;
    offsetMatrices.push_back(ident.Identity());
}

WhirlyKit::Point2f View::unwrapCoordinate(const WhirlyKit::Point2f &pt)
{
    return pt;
}

double View::heightAboveSurface()
{
    return 0.0;
}

Eigen::Vector3d View::eyePos()
{
    return Eigen::Vector3d(0,0,0);
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


double View::currentMapScale(const WhirlyKit::Point2f &frameSize)
{
    //    *height = globeView.heightAboveGlobe;
    //    Point3d localPt = [globeView currentUp];
    //    GeoCoord geoCoord = globeView.coordAdapter->getCoordSystem()->localToGeographic(globeView.coordAdapter->displayToLocal(localPt));
    //    pos->x = geoCoord.lon();  pos->y = geoCoord.lat();
    
//    Point2f frameSize(sceneRenderer.framebufferWidth,sceneRenderer.framebufferHeight);
//    Eigen::Matrix4d modelTrans = [visualView calcFullMatrix];
//    Point3d sp0,sp1;
//    bool sp0Valid = [globeView pointOnSphereFromScreen:CGPointMake(0.0, frameSize.y()/2.0) transform:&modelTrans frameSize:frameSize hit:&sp0 normalized:true];
//    bool sp1Valid = [globeView pointOnSphereFromScreen:CGPointMake(frameSize.x(), frameSize.y()/2.0) transform:&modelTrans frameSize:frameSize hit:&sp1 normalized:true];
//    // Bogus scale at this point
//    if (!sp0Valid || !sp1Valid)
//        return 0.0;
//    sp0 *= EarthRadius;
//    sp1 *= EarthRadius;
//    // Assume the local coordinate are in meters.  WHAT COULD POSSIBLY GO WRONG!
//    double dist = (sp1-sp0).norm();
    
    // This is Mapnik scale:
    // scale_denominator = map_width_in_metres/ (map_width_in_pixels * standardized_pixel_size/*0.28mm*/)
    double scale = (2 * heightAboveSurface() *  tan(fieldOfView/2.0) * EarthRadius) / (frameSize.x() * 0.00096) ;
    return scale;
}

double View::heightForMapScale(double scale,const WhirlyKit::Point2f &frameSize)
{
    double height = (scale * frameSize.x() * 0.00096) / (2 * tan(fieldOfView/2.0) * EarthRadius);
    return height;
}

/*
 S = C*cos(y)/2^(z+8)
 z = log2(C * cos(y) / S) - 8
*/
double View::currentMapZoom(const WhirlyKit::Point2f &frameSize,double latitude)
{
  double mapWidthInMeters = (2 * heightAboveSurface() *  tan(fieldOfView/2.0) * EarthRadius);
  double metersPerPizel = mapWidthInMeters/frameSize.x();
  double zoom = log(EarthRadius * RadToDeg(cos(latitude))/ metersPerPizel)/log(2.0) - 8;
  
  return zoom;
}

Point2d View::screenSizeInDisplayCoords(Point2f &frameSize)
{
    Point2d screenSize(0,0);
    if (frameSize.x() == 0.0 || frameSize.y() == 0.0)
        return screenSize;
    
    screenSize.x() = tan(fieldOfView/2.0) * heightAboveSurface() * 2.0;
    screenSize.y() = screenSize.x() / frameSize.x() * frameSize.y();
    
    return screenSize;
}

/// Add a watcher delegate
void View::addWatcher(ViewWatcher *watcher)
{
    std::lock_guard<std::mutex> guardLock(watcherLock);
    watchers.insert(watcher);
}

/// Remove the given watcher delegate
void View::removeWatcher(ViewWatcher *watcher)
{
    std::lock_guard<std::mutex> guardLock(watcherLock);
    watchers.erase(watcher);
}

void View::runViewUpdates()
{
    // Make a copy so we don't step on watchers being removed
    ViewWatcherSet watchersToRun;
    {
        std::lock_guard<std::mutex> guardLock(watcherLock);
        watchersToRun = watchers;
    }
    for (ViewWatcherSet::iterator it = watchersToRun.begin();
         it != watchersToRun.end(); ++it)
        (*it)->viewUpdated(this);
}

ViewState::ViewState(WhirlyKit::View *view,SceneRenderer *renderer)
{
    modelMatrix = view->calcModelMatrix();
    invModelMatrix = modelMatrix.inverse();
    
    std::vector<Eigen::Matrix4d> offMatrices;
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
        u = std::max(0.0,u);    u = std::min(1.0,u);
    }
    double v = screenPt.y() / frameHeight;
    if (clip)
    {
        v = std::max(0.0,v);    v = std::min(1.0,v);
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
    wkLogLevel(Verbose,"--- ViewState ---");
    wkLogLevel(Verbose,"eyeVec = (%f,%f,%f), eyeVecModel = (%f,%f,%f)",eyeVec.x(),eyeVec.y(),eyeVec.z(),eyeVecModel.x(),eyeVecModel.y(),eyeVecModel.z());
    for (unsigned int mm=0;mm<fullMatrices.size();mm++)
    {
        std::stringstream strStrm;
        for (unsigned int ii=0;ii<16;ii++)
            strStrm << fullMatrices[mm].data()[ii];
        wkLogLevel(Verbose,"fullMatrix = %@",strStrm.str().c_str());
    }
    wkLogLevel(Verbose,"---     ---   ---");
}
    
}
