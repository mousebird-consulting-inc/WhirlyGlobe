/*
 *  GlobeView.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/14/11.
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
#import "GlobeView.h"
#import "WhirlyGeometry.h"
#import "GlobeMath.h"

using namespace WhirlyKit;
using namespace Eigen;

namespace WhirlyGlobe
{

GlobeView::GlobeView(WhirlyKit::CoordSystemDisplayAdapter *inCoordAdapter)
{
    coordAdapter = inCoordAdapter;
    rotQuat = Eigen::AngleAxisd(0.0f,Vector3d(0.0f,0.0f,1.0f));
    coordAdapter = &fakeGeoC;
    defaultNearPlane = nearPlane;
    defaultFarPlane = farPlane;
    // This will get you down to r17 in the usual tile sets
    absoluteMinNearPlane = 0.000001;
    absoluteMinFarPlane = 0.001;
    absoluteMinHeight = 0.000002;
    heightInflection = 0.011;
    heightAboveGlobe = 1.1;
    tilt = 0.0;
}

//    double absoluteMinHeight;
//    double heightInflection;
//    double defaultNearPlane;
//    double absoluteMinNearPlane;
//    double defaultFarPlane;
//    double absoluteMinFarPlane;
//    
//protected:
//    void privateSetHeightAboveGlobe(double newH,bool updateWatchers);
//    
//    /// The globe has a radius of 1.0 so 1.0 + heightAboveGlobe is the offset from the middle of the globe
//    double heightAboveGlobe;
//    /// Quaternion used for rotation from origin state
//    Eigen::Quaterniond rotQuat;
//    /// The view can have a tilt.  0 is straight down.  PI/2 is looking to the horizon.
//    double tilt;

GlobeView::GlobeView(const GlobeView &that)
: View(that), absoluteMinHeight(that.absoluteMinHeight), heightInflection(that.heightInflection), defaultNearPlane(that.defaultNearPlane),
    absoluteMinNearPlane(that.absoluteMinNearPlane), defaultFarPlane(that.defaultFarPlane), absoluteMinFarPlane(that.absoluteMinFarPlane),
    heightAboveGlobe(that.heightAboveGlobe), rotQuat(that.rotQuat), tilt(that.tilt)
{
}
    
GlobeView::~GlobeView()
{
}

void GlobeView::setRotQuat(Eigen::Quaterniond newRotQuat)
{
    setRotQuat(newRotQuat,true);
}

// Set the new rotation, but also keep track of when we did it
void GlobeView::setRotQuat(Eigen::Quaterniond newRotQuat,bool updateWatchers)
{
    lastChangedTime = TimeGetCurrent();
    rotQuat = newRotQuat;
    if (updateWatchers)
        runViewUpdates();
}

void GlobeView::setTilt(double newTilt)
{
    tilt = newTilt;
}
	
double GlobeView::minHeightAboveGlobe()
{
    if (continuousZoom)
        return absoluteMinHeight;
    else
        return 1.01*nearPlane;
}

double GlobeView::heightAboveSurface()
{
    return heightAboveGlobe;
}
	
double GlobeView::maxHeightAboveGlobe()
{
    return (farPlane - 1.0);
}
	
double GlobeView::calcEarthZOffset()
{
	float minH = minHeightAboveGlobe();
	if (heightAboveGlobe < minH)
		return 1.0+minH;
	
	float maxH = maxHeightAboveGlobe();
	if (heightAboveGlobe > maxH)
		return 1.0+maxH;
	
	return 1.0 + heightAboveGlobe;
}

void GlobeView::setHeightAboveGlobe(double newH)
{
    privateSetHeightAboveGlobe(newH,true);
}

void GlobeView::setHeightAboveGlobe(double newH,bool updateWatchers)
{
    privateSetHeightAboveGlobe(newH,updateWatchers);
}

void GlobeView::setHeightAboveGlobeNoLimits(double newH,bool updateWatchers)
{
	heightAboveGlobe = newH;
    
    // If we get down below the inflection point we'll start messing
    //  with the field of view.  Not ideal, but simple.
    if (continuousZoom)
    {
        if (heightAboveGlobe < heightInflection)
        {
            double t = 1.0 - (heightInflection - heightAboveGlobe) / (heightInflection - absoluteMinHeight);
            nearPlane = t * (defaultNearPlane-absoluteMinNearPlane) + absoluteMinNearPlane;
            farPlane = t * (defaultFarPlane-absoluteMinFarPlane) + absoluteMinFarPlane;
        } else {
            nearPlane = defaultNearPlane;
            farPlane = defaultFarPlane;
        }
		imagePlaneSize = nearPlane * tan(fieldOfView / 2.0);
    }
    
    lastChangedTime = TimeGetCurrent();
    
    if (updateWatchers)
        runViewUpdates();
}

// Set the height above the globe, but constrain it
// Also keep track of when we did it
void GlobeView::privateSetHeightAboveGlobe(double newH,bool updateWatchers)
{
	double minH = minHeightAboveGlobe();
	heightAboveGlobe = std::max(newH,minH);
    
	double maxH = maxHeightAboveGlobe();
	heightAboveGlobe = std::min(heightAboveGlobe,maxH);

    // If we get down below the inflection point we'll start messing
    //  with the field of view.  Not ideal, but simple.
    if (continuousZoom)
    {
        if (heightAboveGlobe < heightInflection)
        {
            double t = 1.0 - (heightInflection - heightAboveGlobe) / (heightInflection - absoluteMinHeight);
            nearPlane = t * (defaultNearPlane-absoluteMinNearPlane) + absoluteMinNearPlane;
//            farPlane = t * (defaultFarPlane-absoluteMinFarPlane) + absoluteMinFarPlane;
        } else {
            nearPlane = defaultNearPlane;
//            farPlane = defaultFarPlane;
        }
		imagePlaneSize = nearPlane * tan(fieldOfView / 2.0);
    }
    
    lastChangedTime = TimeGetCurrent();
    
    if (updateWatchers)
       runViewUpdates();
}
	
Eigen::Matrix4d GlobeView::calcModelMatrix()
{
    // Note: Porting.  Have a lock in the iOS version

    Eigen::Affine3d trans(Eigen::Translation3d(0,0,-calcEarthZOffset()));
	Eigen::Affine3d rot(rotQuat);
	
	return (trans * rot).matrix();
}

Eigen::Matrix4d GlobeView::calcViewMatrix()
{
    // Note: Porting.  Have a lock in the iOS version

    Eigen::Quaterniond selfRotPitch(AngleAxisd(-tilt, Vector3d::UnitX()));
    
    return ((Affine3d)selfRotPitch).matrix();
}

Vector3d GlobeView::currentUp()
{
	Eigen::Matrix4d modelMat = calcModelMatrix().inverse();
	
	Vector4d newUp = modelMat * Vector4d(0,0,1,0);
	return Vector3d(newUp.x(),newUp.y(),newUp.z());
}

Vector3d GlobeView::prospectiveUp(Eigen::Quaterniond &prospectiveRot)
{
    Eigen::Affine3d rot(prospectiveRot);
    Eigen::Matrix4d modelMat = rot.inverse().matrix();
    Vector4d newUp = modelMat *Vector4d(0,0,1,0);
    return Vector3d(newUp.x(),newUp.y(),newUp.z());
}
	
bool GlobeView::pointOnSphereFromScreen(const Point2f &pt,const Eigen::Matrix4d *transform,const Point2f &frameSize,Point3d *hit,bool normalized)
{
	// Back project the point from screen space into model space
	Point3d screenPt = pointUnproject(Point2f(pt.x(),pt.y()),frameSize.x(),frameSize.y(),true);
	
	// Run the screen point and the eye point (origin) back through
	//  the model matrix to get a direction and origin in model space
	Eigen::Matrix4d modelTrans = *transform;
	Matrix4d invModelMat = modelTrans.inverse();
	Point3d eyePt(0,0,0);
	Vector4d modelEye = invModelMat * Vector4d(eyePt.x(),eyePt.y(),eyePt.z(),1.0);
	Vector4d modelScreenPt = invModelMat * Vector4d(screenPt.x(),screenPt.y(),screenPt.z(),1.0);
	
	// Now intersect that with a unit sphere to see where we hit
	Vector4d dir4 = modelScreenPt - modelEye;
	Vector3d dir(dir4.x(),dir4.y(),dir4.z());
        double t;
	if (IntersectUnitSphere(Vector3d(modelEye.x(),modelEye.y(),modelEye.z()), dir, *hit, &t) && t > 0.0)
		return true;
	
	// We need the closest pass, if that didn't work out
    if (normalized)
    {
	Vector3d orgDir(-modelEye.x(),-modelEye.y(),-modelEye.z());
	orgDir.normalize();
	dir.normalize();
	Vector3d tmpDir = orgDir.cross(dir);
	Vector3d resVec = dir.cross(tmpDir);
	*hit = -resVec.normalized();
    } else {
        double len2 = dir.squaredNorm();
        double top = dir.dot(Vector3d(modelScreenPt.x(),modelScreenPt.y(),modelScreenPt.z()));
        double t = 0.0;
        if (len2 > 0.0)
            t = top/len2;
        *hit = Vector3d(modelEye.x(),modelEye.y(),modelEye.z()) + dir*t;
    }
	
	return false;
}

Point2f GlobeView::pointOnScreenFromSphere(const Point3d &worldLoc,const Eigen::Matrix4d *transform,const Point2f &frameSize)
{
    // Run the model point through the model transform (presumably what they passed in)
    Eigen::Matrix4d modelTrans = *transform;
    Matrix4d modelMat = modelTrans;
    Vector4d screenPt = modelMat * Vector4d(worldLoc.x(),worldLoc.y(),worldLoc.z(),1.0);
    screenPt.x() /= screenPt.w();  screenPt.y() /= screenPt.w();  screenPt.z() /= screenPt.w();

    // Intersection with near gives us the same plane as the screen 
    Point3d ray;
    ray.x() = screenPt.x() / screenPt.w();  ray.y() = screenPt.y() / screenPt.w();  ray.z() = screenPt.z() / screenPt.w();
    ray *= -nearPlane/ray.z();

    // Now we need to scale that to the frame
    Point2d ll,ur;
    double near,far;
    calcFrustumWidth(frameSize.x(),frameSize.y(),ll,ur,near,far);
    double u = (ray.x() - ll.x()) / (ur.x() - ll.x());
    double v = (ray.y() - ll.y()) / (ur.y() - ll.y());
    v = 1.0 - v;
    
    Point2f retPt;
    retPt.x() = u * frameSize.x();
    retPt.y() = v * frameSize.y();
    
    return retPt;
}

// Note: Porting.  Need to wrap 'this' in an object
//void GlobeView::setDelegate(NSObject<WhirlyGlobeAnimationDelegate> *inDelegate)
//{
//    if (!inDelegate)
//        [[NSNotificationCenter defaultCenter] postNotificationName:kWKViewAnimationEnded object:self];
//    else {
//        [[NSNotificationCenter defaultCenter] postNotificationName:kWKViewAnimationStarted object:self];
//    }
//    
//    delegate = inDelegate;
//}

void GlobeView::cancelAnimation()
{
    // Note: Porting.  Need to wrap 'this' in an object
//    if (delegate)
//        [[NSNotificationCenter defaultCenter] postNotificationName:kWKViewAnimationEnded object:self];

    // Note: Porting
//    delegate = nil;
}

// Run the rotation animation
void GlobeView::animate()
{
    // Note: Porting
//    if (delegate)
//        [delegate updateView:this];
}

// Calculate the Z buffer resolution
float GlobeView::calcZbufferRes()
{
    float delta;
//    int numBits = 16;
//    float testZ = 1.5;  // Should only need up to 1.0 away, actually
//    delta = testZ * testZ / ( nearPlane * (1<<numBits - 1));
    
    // Note: Not right
    delta = 0.0001;
    
    return delta;
}

// Construct a rotation to the given location
//  and return it.  Doesn't actually do anything yet.
Eigen::Quaterniond GlobeView::makeRotationToGeoCoord(const WhirlyKit::GeoCoord &worldCoord,bool northUp)
{
    Point3d worldLoc = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal3d(worldCoord));
    
    // Let's rotate to where they tapped over a 1sec period
    Vector3d curUp = currentUp();
    
    // The rotation from where we are to where we tapped
    Eigen::Quaterniond endRot;
    endRot = QuatFromTwoVectors(worldLoc,curUp);
    Eigen::Quaterniond curRotQuat = rotQuat;
    Eigen::Quaterniond newRotQuat = curRotQuat * endRot;
    
    if (northUp)
    {
        // We'd like to keep the north pole pointed up
        // So we look at where the north pole is going
        Vector3d northPole = (newRotQuat * Vector3d(0,0,1)).normalized();
        if (northPole.y() != 0.0)
        {
            // Then rotate it back on to the YZ axis
            // This will keep it upward
            float ang = atan(northPole.x()/northPole.y());
            // However, the pole might be down now
            // If so, rotate it back up
            if (northPole.y() < 0.0)
                ang += M_PI;
            Eigen::AngleAxisd upRot(ang,worldLoc);
            newRotQuat = newRotQuat * upRot;
        }
    }
    
    return newRotQuat;
}

Eigen::Vector3d GlobeView::eyePos()
{
	Eigen::Matrix4d modelMat = calcModelMatrix().inverse();
	
	Vector4d newUp = modelMat * Vector4d(0,0,1,1);
	return Vector3d(newUp.x(),newUp.y(),newUp.z());
}

}
