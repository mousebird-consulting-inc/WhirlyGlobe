/*
 *  WhirlyKitView.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/9/12.
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

#import "Platform.h"
#import "WhirlyVector.h"
#import "WhirlyKitView.h"
#import "WhirlyGeometry.h"
#import "FlatMath.h"

using namespace Eigen;

namespace WhirlyKit
{

View::View()
{
    fieldOfView = 60.0 / 360.0 * 2 * (float)M_PI;  // 60 degree field of view
    nearPlane = 0.001;
    imagePlaneSize = nearPlane * tanf(fieldOfView / 2.0);
    farPlane = 10.0;
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

void View::getOffsetMatrices(std::vector<Eigen::Matrix4d> &offsetMatrices,const WhirlyKit::Point2f &frameBufferSize)
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
