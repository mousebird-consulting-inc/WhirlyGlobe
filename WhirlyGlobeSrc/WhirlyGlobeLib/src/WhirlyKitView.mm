/*
 *  WhirlyKitView.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/9/12.
 *  Copyright 2011-2015 mousebird consulting
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

#import "WhirlyVector.h"
#import "WhirlyKitView.h"
#import "WhirlyGeometry.h"
#import "FlatMath.h"

using namespace WhirlyKit;
using namespace Eigen;

@implementation WhirlyKitView
{
    /// Called when positions are updated
    WhirlyKitViewWatcherDelegateSet watchDelegates;
}

- (id)init
{
	if ((self = [super init]))
    {
        _fieldOfView = 60.0 / 360.0 * 2 * (float)M_PI;  // 60 degree field of view
		_nearPlane = 0.001;
		_imagePlaneSize = _nearPlane * tanf(_fieldOfView / 2.0);
		_farPlane = 10.0;
        _lastChangedTime = CFAbsoluteTimeGetCurrent();
        _continuousZoom = false;
    }
    
    return self;
}

- (id)initWithView:(WhirlyKitView *)inView
{
    self = [super init];
    _fieldOfView = inView->_fieldOfView;
    _nearPlane = inView->_nearPlane;
    _imagePlaneSize = inView->_imagePlaneSize;
    _farPlane = inView->_farPlane;
    _lastChangedTime = inView->_lastChangedTime;
    _continuousZoom = inView->_continuousZoom;
    
    return self;
}

- (void)calcFrustumWidth:(unsigned int)frameWidth height:(unsigned int)frameHeight ll:(Point2d &)ll ur:(Point2d &)ur near:(double &)near far:(double &)far
{
	ll.x() = -_imagePlaneSize;
	ur.x() = _imagePlaneSize;
	double ratio =  ((double)frameHeight / (double)frameWidth);
	ll.y() = -_imagePlaneSize * ratio;
	ur.y() = _imagePlaneSize * ratio ;
	near = _nearPlane;
	far = _farPlane;
}

- (void)cancelAnimation
{
}

- (void)animate
{
}

- (float)calcZbufferRes
{
    return 1.0;
}

/// Generate the model view matrix for use by OpenGL.
- (Eigen::Matrix4d)calcModelMatrix
{
    Eigen::Matrix4d ident = ident.Identity();
    return ident;
}

- (Eigen::Matrix4d)calcViewMatrix
{
    Eigen::Matrix4d ident = ident.Identity();
    return ident;
}

- (Eigen::Matrix4d)calcFullMatrix
{
    return [self calcViewMatrix] * [self calcModelMatrix];
}

- (Eigen::Matrix4d)calcProjectionMatrix:(Point2f)frameBufferSize margin:(float)margin
{
	GLfloat near=0,far=0;
	Point2d frustLL,frustUR;
	frustLL.x() = -_imagePlaneSize * (1.0 + margin);
	frustUR.x() = _imagePlaneSize * (1.0 + margin);
	double ratio =  ((double)frameBufferSize.y() / (double)frameBufferSize.x());
	frustLL.y() = -_imagePlaneSize * ratio * (1.0 + margin);
	frustUR.y() = _imagePlaneSize * ratio * (1.0 + margin);
	near = _nearPlane;
	far = _farPlane;
    
    
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

- (void) getOffsetMatrices:(std::vector<Eigen::Matrix4d> &)offsetMatrices frameBuffer:(WhirlyKit::Point2f)frameBufferSize;
{
    Eigen::Matrix4d ident;
    offsetMatrices.push_back(ident.Identity());
}

- (WhirlyKit::Point2f)unwrapCoordinate:(WhirlyKit::Point2f)pt
{
    return pt;
}

- (double)heightAboveSurface
{
    return 0.0;
}

- (Point3d)pointUnproject:(Point2f)screenPt width:(unsigned int)frameWidth height:(unsigned int)frameHeight clip:(bool)clip
{
	Point2d ll,ur;
	double near,far;
	[self calcFrustumWidth:frameWidth height:frameHeight ll:ll ur:ur near:near far:far];
	
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

- (double)currentMapScale:(WhirlyKit::Point2f &)frameSize
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
    double scale = (2 * self.heightAboveSurface *  tan(_fieldOfView/2.0) * EarthRadius) / (frameSize.x() * 0.00096) ;
    return scale;
}

- (double)heightForMapScale:(double)scale frame:(WhirlyKit::Point2f &)frameSize
{
    double height = (scale * frameSize.x() * 0.00096) / (2 * tan(_fieldOfView/2.0) * EarthRadius);
    return height;
}

/*
 S = C*cos(y)/2^(z+8)
 z = log2(C * cos(y) / S) - 8
*/
- (double)currentMapZoom:(WhirlyKit::Point2f &)frameSize latitude:(double)latitude
{
  double mapWidthInMeters = (2 * self.heightAboveSurface *  tan(_fieldOfView/2.0) * EarthRadius);
  double metersPerPizel = mapWidthInMeters/frameSize.x();
  double zoom = log2(EarthRadius * RadToDeg(cos(latitude))/ metersPerPizel) - 8;
  
  return zoom;
}

- (WhirlyKit::Point2d)screenSizeInDisplayCoords:(WhirlyKit::Point2f &)frameSize
{
    Point2d screenSize(0,0);
    if (frameSize.x() == 0.0 || frameSize.y() == 0.0)
        return screenSize;
    
    screenSize.x() = tan(_fieldOfView/2.0) * self.heightAboveSurface * 2.0;
    screenSize.y() = screenSize.x() / frameSize.x() * frameSize.y();
    
    return screenSize;
}

/// Add a watcher delegate
- (void)addWatcherDelegate:(NSObject<WhirlyKitViewWatcherDelegate> *)delegate
{
    watchDelegates.insert(delegate);
}

/// Remove the given watcher delegate
- (void)removeWatcherDelegate:(NSObject<WhirlyKitViewWatcherDelegate> *)delegate
{
    watchDelegates.erase(delegate);
}

- (void)runViewUpdates
{
    for (WhirlyKitViewWatcherDelegateSet::iterator it = watchDelegates.begin();
         it != watchDelegates.end(); ++it)
        [(*it) viewUpdated:self];    
}

@end
