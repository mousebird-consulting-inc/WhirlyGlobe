/*
 *  MaplyView.h
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

#import "Platform.h"
#import "MaplyView.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace Maply
{

MapView::MapView(WhirlyKit::CoordSystemDisplayAdapter *inCoordAdapter)
{
    coordAdapter = inCoordAdapter;
    fieldOfView = 60.0 / 360.0 * 2 * (float)M_PI;  // 60 degree field of view
    nearPlane = 0.00001;
    imagePlaneSize = nearPlane * tanf(fieldOfView / 2.0);
    farPlane = 5.0;
    lastChangedTime = TimeGetCurrent();
    continuousZoom = false;
    loc = Point3d(0,0,4);
    rotAngle = 0.0;
    wrap = true;
}
    
MapView::MapView(const MapView &that)
    : loc(that.loc), rotAngle(that.rotAngle)
{
}
    
MapView::~MapView()
{    
}

// Note: Porting
//void MapView::setDelegate(NSObject<MaplyAnimationDelegate> *inDelegate)
//{
//    if (!delegate)
//        [[NSNotificationCenter defaultCenter] postNotificationName:kWKViewAnimationEnded object:self];
//    else {
//        [[NSNotificationCenter defaultCenter] postNotificationName:kWKViewAnimationStarted object:self];
//    }
//    
//    delegate = inDelegate;
//}

void MapView::cancelAnimation()
{
    // Note: Porting
//    if (_delegate)
//        [[NSNotificationCenter defaultCenter] postNotificationName:kWKViewAnimationEnded object:self];
//    
//    delegate = nil;
}

void MapView::animate()
{
    // Note: Porting
//    if (delegate)
//        [delegate updateView:this];
}

float MapView::calcZbufferRes()
{
    // Note: Not right
    double delta = 0.0001;
    
    return delta;
}

Eigen::Matrix4d MapView::calcModelMatrix()
{
    Eigen::Affine3d trans(Eigen::Translation3d(-loc.x(),-loc.y(),-loc.z()));
//    Eigen::Affine3d rot(Eigen::AngleAxisd(-_rotAngle, Vector3d::UnitZ()).toRotationMatrix());
//    return (rot * trans).matrix();
    return trans.matrix();
}

Eigen::Matrix4d MapView::calcViewMatrix()
{
    Eigen::Affine3d rot(Eigen::AngleAxisd(-rotAngle, Vector3d::UnitZ()).toRotationMatrix());
    return rot.matrix();
}

void MapView::getOffsetMatrices(std::vector<Eigen::Matrix4d> &offsetMatrices,const WhirlyKit::Point2f &frameBufferSize)
{
    Point3f ll,ur;
    if (wrap && coordAdapter && coordAdapter->getBounds(ll, ur))
    {
        // Figure out where we are, first off
        GeoCoord geoLL = coordAdapter->getCoordSystem()->localToGeographic(ll);
        GeoCoord geoUR = coordAdapter->getCoordSystem()->localToGeographic(ur);
        float spanX = geoUR.x()-geoLL.x();
        float offX = loc.x()-geoLL.x();
        int num = floorf(offX/spanX);
        std::vector<int> nums;
        nums.push_back(num);
        nums.push_back(num-1);
        nums.push_back(num+1);
        
        float localSpanX = ur.x()-ll.x();
        
        // See if the framebuffer lands in any of the potential matrices
        Eigen::Matrix4d modelTrans = calcViewMatrix() * calcModelMatrix();
        Mbr screenMbr;
        screenMbr.addPoint(Point2f(-1.0,-1.0));
        screenMbr.addPoint(Point2f(1.0,1.0));
        Matrix4d projMat = calcProjectionMatrix(frameBufferSize,0.0);
        for (unsigned int ii=0;ii<nums.size();ii++)
        {
            int thisNum = nums[ii];
            Eigen::Affine3d offsetMat(Eigen::Translation3d(thisNum*localSpanX,0.0,0.0));
            Eigen::Matrix4d testMat = projMat * modelTrans;
            Point3d testPts[4];
            testPts[0] = Point3d(thisNum*localSpanX+ll.x(),ll.y(),0.0);
            testPts[1] = Point3d((thisNum+1)*localSpanX+ll.x(),ll.y(),0.0);
            testPts[2] = Point3d((thisNum+1)*localSpanX+ll.x(),ur.y(),0.0);
            testPts[3] = Point3d(thisNum*localSpanX+ll.x(),ur.y(),0.0);
            Mbr testMbr;
            for (unsigned int jj=0;jj<4;jj++)
            {
                Vector4d screenPt = testMat * Vector4d(testPts[jj].x(),testPts[jj].y(),testPts[jj].z(),1.0);
                screenPt /= screenPt.w();
                testMbr.addPoint(Point2f(screenPt.x(),screenPt.y()));
            }
            if (testMbr.overlaps(screenMbr))
                offsetMatrices.push_back(offsetMat.matrix());
        }

        // Don't know why this would happen, but let's not tempt fate
        if (offsetMatrices.empty())
            offsetMatrices.push_back(Matrix4d::Identity());
    } else {
        // Just pass back the identity matrix
        Eigen::Matrix4d ident;
        offsetMatrices.push_back(ident.Identity());
    }
}

WhirlyKit::Point2f MapView::unwrapCoordinate(const WhirlyKit::Point2f &inPt)
{
    Point2f pt = inPt;
    
    if (wrap)
    {
        Point3f ll,ur;
        if (coordAdapter->getBounds(ll, ur))
        {
            GeoCoord geoLL = coordAdapter->getCoordSystem()->localToGeographic(ll);
            GeoCoord geoUR = coordAdapter->getCoordSystem()->localToGeographic(ur);
            float spanX = geoUR.x()-geoLL.x();
            float offX = pt.x()-geoLL.x();
            int num = floorf(offX/spanX);
            pt.x() += -num * spanX;
        }
    }
    
    return pt;
}

double MapView::heightAboveSurface()
{
    return loc.z();
}

double MapView::minHeightAboveSurface()
{
    return nearPlane;
}

double MapView::maxHeightAboveSurface()
{
    return farPlane;
}

void MapView::setLoc(WhirlyKit::Point3d newLoc)
{
    setLoc(newLoc,true);
}

void MapView::setLoc(WhirlyKit::Point3d &newLoc,bool runUpdates)
{
    loc = newLoc;
    if (runUpdates)
        runViewUpdates();
}

void MapView::setRotAngle(double newRotAngle)
{
    rotAngle = newRotAngle;
    runViewUpdates();
}

Eigen::Matrix4d MapView::calcFullMatrix()
{
    return calcViewMatrix() * calcModelMatrix();
}

bool MapView::pointOnPlaneFromScreen(Point2f pt,const Eigen::Matrix4d *transform,const Point2f &frameSize,Point3d *hit,bool clip)
{
    // Back Project the screen point into model space
    Point3d screenPt = pointUnproject(Point2f(pt.x(),pt.y()),frameSize.x(),frameSize.y(),clip);
    
    // Run the screen point and the eye point (origin) back through
    //  the model matrix to get a direction and origin in model space
    Eigen::Matrix4d modelTrans = *transform;
    Matrix4d invModelMat = modelTrans.inverse();
    Point3d eyePt(0,0,0);
    Vector4d modelEye = invModelMat * Vector4d(eyePt.x(),eyePt.y(),eyePt.z(),1.0);
    Vector4d modelScreenPt = invModelMat * Vector4d(screenPt.x(),screenPt.y(),screenPt.z(),1.0);
    
    // Now intersect with the plane at (0,0)
    // Okay, this is kind of overkill
    Vector4d dir4 = modelScreenPt - modelEye;
    Vector3d dir(dir4.x(),dir4.y(),dir4.z());
    
    if (dir.z() == 0.0)
        return false;
    dir.normalize();
    double t = - modelEye.z() / dir.z();
    *hit = Vector3d(modelEye.x(),modelEye.y(),modelEye.z()) + dir * t;
    
    return true;
}

Point2f MapView::pointOnScreenFromPlane(const Point3d &inWorldLoc,const Eigen::Matrix4d *transform,const Point2f &frameSize)
{
    Point3d worldLoc(inWorldLoc.x(),inWorldLoc.y(),inWorldLoc.z());
    
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

}
