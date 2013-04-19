/*
 *  GlobeView.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/14/11.
 *  Copyright 2011-2012 mousebird consulting
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
#import "GlobeView.h"
#import "WhirlyGeometry.h"
#import "GlobeMath.h"

using namespace WhirlyKit;
using namespace Eigen;

@interface WhirlyGlobeView()
{
    // These are all for continuous zoom mode
    double absoluteMinHeight;
    double heightInflection;
    double defaultNearPlane;
    double absoluteMinNearPlane;
    double defaultFarPlane;
    double absoluteMinFarPlane;
}
@end

@implementation WhirlyGlobeView

@synthesize heightAboveGlobe;
@synthesize delegate;
@synthesize rotQuat;
@synthesize tilt;

- (id)init
{
	if ((self = [super init]))
	{
		rotQuat = Eigen::AngleAxisd(0.0f,Vector3d(0.0f,0.0f,1.0f));
        coordAdapter = new FakeGeocentricDisplayAdapter();
        defaultNearPlane = nearPlane;
        defaultFarPlane = farPlane;
        // This will get you down to r17 in the usual tile sets
        absoluteMinNearPlane = 0.00001;
        absoluteMinFarPlane = 0.001;
        absoluteMinHeight = 0.00005;
        heightInflection = 0.011;
		self.heightAboveGlobe = 1.1;
        tilt = 0.0;
	}
	
	return self;
}

- (void)dealloc
{
    if (coordAdapter)
        delete coordAdapter;
    coordAdapter = nil;
}

// Set the new rotation, but also keep track of when we did it
- (void)setRotQuat:(Eigen::Quaterniond)newRotQuat
{
    lastChangedTime = CFAbsoluteTimeGetCurrent();
    rotQuat = newRotQuat;
    [self runViewUpdates];
}

- (void)setTilt:(double)newTilt
{
    tilt = newTilt;
}
	
- (double)minHeightAboveGlobe
{
    if (continuousZoom)
        return absoluteMinHeight;
    else
        return 1.01*nearPlane;
}

- (double)heightAboveSurface
{
    return self.heightAboveGlobe;
}
	
- (double)maxHeightAboveGlobe
{
    return (farPlane - 1.0);
}
	
- (double)calcEarthZOffset
{
	float minH = [self minHeightAboveGlobe];
	if (heightAboveGlobe < minH)
		return 1.0+minH;
	
	float maxH = [self maxHeightAboveGlobe];
	if (heightAboveGlobe > maxH)
		return 1.0+maxH;
	
	return 1.0 + heightAboveGlobe;
}

// Set the height above the globe, but constrain it
// Also keep track of when we did it
- (void)setHeightAboveGlobe:(double)newH
{
	double minH = [self minHeightAboveGlobe];
	heightAboveGlobe = std::max(newH,minH);
    
	double maxH = [self maxHeightAboveGlobe];
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
        

    lastChangedTime = CFAbsoluteTimeGetCurrent();
    
    [self runViewUpdates];
}
	
- (Eigen::Matrix4d)calcModelMatrix
{
	Eigen::Affine3d trans(Eigen::Translation3d(0,0,-[self calcEarthZOffset]));
	Eigen::Affine3d rot(rotQuat);
	
	return (trans * rot).matrix();
}

- (Eigen::Matrix4d)calcViewMatrix
{
    Eigen::Quaterniond selfRotPitch(AngleAxisd(-tilt, Vector3d::UnitX()));
    
    return ((Affine3d)selfRotPitch).matrix();
}

- (Vector3d)currentUp
{
	Eigen::Matrix4d modelMat = [self calcModelMatrix].inverse();
	
	Vector4d newUp = modelMat * Vector4d(0,0,1,0);
	return Vector3d(newUp.x(),newUp.y(),newUp.z());
}

+ (Vector3d)prospectiveUp:(Eigen::Quaterniond &)prospectiveRot
{
    Eigen::Affine3d rot(prospectiveRot);
    Eigen::Matrix4d modelMat = rot.inverse().matrix();
    Vector4d newUp = modelMat *Vector4d(0,0,1,0);
    return Vector3d(newUp.x(),newUp.y(),newUp.z());
}
	
- (bool)pointOnSphereFromScreen:(CGPoint)pt transform:(const Eigen::Matrix4d *)transform frameSize:(const Point2f &)frameSize hit:(Point3d *)hit
{
	// Back project the point from screen space into model space
	Point3d screenPt = [self pointUnproject:Point2f(pt.x,pt.y) width:frameSize.x() height:frameSize.y() clip:true];
	
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
	if (IntersectUnitSphere(Vector3d(modelEye.x(),modelEye.y(),modelEye.z()), dir, *hit))
		return true;
	
	// We need the closest pass, if that didn't work out
	Vector3d orgDir(-modelEye.x(),-modelEye.y(),-modelEye.z());
	orgDir.normalize();
	dir.normalize();
	Vector3d tmpDir = orgDir.cross(dir);
	Vector3d resVec = dir.cross(tmpDir);
	*hit = -resVec.normalized();
	
	return false;
}

- (CGPoint)pointOnScreenFromSphere:(const Point3d &)worldLoc transform:(const Eigen::Matrix4d *)transform frameSize:(const Point2f &)frameSize
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
    [self calcFrustumWidth:frameSize.x() height:frameSize.y() ll:ll ur:ur near:near far:far];
    double u = (ray.x() - ll.x()) / (ur.x() - ll.x());
    double v = (ray.y() - ll.y()) / (ur.y() - ll.y());
    v = 1.0 - v;
    
    CGPoint retPt;
    retPt.x = u * frameSize.x();
    retPt.y = v * frameSize.y();
    
    return retPt;
}

- (void)cancelAnimation
{
    self.delegate = nil;
}

// Run the rotation animation
- (void)animate
{
    if (delegate)
        [delegate updateView:self];
}

// Calculate the Z buffer resolution
- (float)calcZbufferRes
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
- (Eigen::Quaterniond) makeRotationToGeoCoord:(const GeoCoord &)worldCoord keepNorthUp:(BOOL)northUp
{
    Point3d worldLoc = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal3d(worldCoord));
    
    // Let's rotate to where they tapped over a 1sec period
    Vector3d curUp = [self currentUp];
    
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

- (Eigen::Quaterniond) makeRotationToGeoCoordd:(const GeoCoord &)worldCoord keepNorthUp:(BOOL)northUp
{
    Point3d worldLoc = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal3d(worldCoord));
    
    // Let's rotate to where they tapped over a 1sec period
    Vector3d curUp = [self currentUp];
    
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

- (Eigen::Vector3d)eyePos
{
	Eigen::Matrix4d modelMat = [self calcModelMatrix].inverse();
	
	Vector4d newUp = modelMat * Vector4d(0,0,1,1);
	return Vector3d(newUp.x(),newUp.y(),newUp.z());
}

@end
