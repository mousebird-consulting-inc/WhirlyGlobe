/*  MaplyView.cpp
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
#import "MaplyView.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace Maply
{

MapView::MapView(WhirlyKit::CoordSystemDisplayAdapter *inCoordAdapter)
{
    coordAdapter = inCoordAdapter;
    continuousZoom = true;
    absoluteMinNearPlane = 0.000001;
    absoluteMinFarPlane = 0.001;
    absoluteMinHeight = 0.000002;
    heightInflection = 0.011;
    loc = Point3d(0,0,4);
    rotAngle = 0.0;
    wrap = true;

    defaultNearPlane = getNearPlane();
    defaultFarPlane = getFarPlane();
}
    
MapView::MapView(const MapView &that) :
    View(that), loc(that.loc), rotAngle(that.rotAngle), wrap(that.wrap),
    absoluteMinHeight(that.absoluteMinHeight), heightInflection(that.heightInflection),
    defaultNearPlane(that.defaultNearPlane), absoluteMinNearPlane(that.absoluteMinNearPlane),
    defaultFarPlane(that.defaultFarPlane), absoluteMinFarPlane(that.absoluteMinFarPlane)
{
}
    
float MapView::calcZbufferRes()
{
    // Note: Not right
    constexpr double delta = 0.0001;
    
    return delta;
}

Eigen::Matrix4d MapView::calcModelMatrix() const
{
    Point3d scale = coordAdapter->getScale();
    Eigen::Affine3d trans(Eigen::Translation3d(-loc.x()*scale.x(),-loc.y()*scale.y(),-loc.z()*scale.z()));
//    Eigen::Affine3d rot(Eigen::AngleAxisd(-_rotAngle, Vector3d::UnitZ()).toRotationMatrix());
//    return (rot * trans).matrix();
    return trans.matrix();
}

Eigen::Matrix4d MapView::calcViewMatrix() const
{
    Eigen::Affine3d rot(Eigen::AngleAxisd(-rotAngle, Vector3d::UnitZ()).toRotationMatrix());
    return rot.matrix();
}

void MapView::getOffsetMatrices(Matrix4dVector &offsetMatrices,const WhirlyKit::Point2f &frameBufferSize,float bufferSizeX) const
{
    const Point3d scale = coordAdapter->getScale();
    
    Point3f ll,ur;
    if (wrap && coordAdapter && coordAdapter->getBounds(ll, ur))
    {
        // Figure out where we are, first off
        const GeoCoord geoLL = coordAdapter->getCoordSystem()->localToGeographic(ll);
        const GeoCoord geoUR = coordAdapter->getCoordSystem()->localToGeographic(ur);
        const float spanX = geoUR.x()-geoLL.x();
        const float offX = loc.x()*scale.x()-geoLL.x();
        const auto num = (int)floor(offX/spanX);
        const float localSpanX = ur.x()-ll.x();
        
        // See if the framebuffer lands in any of the potential matrices
        const Matrix4d modelTrans = calcViewMatrix() * calcModelMatrix();
        const Matrix4d projMat = calcProjectionMatrix(frameBufferSize,0.0);
        const Matrix4d testMat = projMat * modelTrans;
        const MbrD screenMbr({ -1.0, -1.0 }, { 1.0, 1.0 });

        for (int thisNum : { num, num - 1, num + 1 })
        {
            const Affine3d offsetMat(Translation3d(thisNum*localSpanX,0.0,0.0));
            const Point3d testPts[4] = {
                {  thisNum      * localSpanX + ll.x() - bufferSizeX, ll.y(), 0.0 },
                { (thisNum + 1) * localSpanX + ll.x() + bufferSizeX, ll.y(), 0.0 },
                { (thisNum + 1) * localSpanX + ll.x() + bufferSizeX, ur.y(), 0.0 },
                {  thisNum      * localSpanX + ll.x() - bufferSizeX, ur.y(), 0.0 },
            };
            MbrD testMbr;
            for (unsigned int jj=0;jj<4;jj++)
            {
                testMbr.addPoint(Slice(Clip(Point4d(testMat * Pad(testPts[jj], 1.0)))));
            }
            if (testMbr.overlaps(screenMbr))
            {
                offsetMatrices.push_back(offsetMat.matrix());
            }
        }
    }

    if (offsetMatrices.empty())
    {
        offsetMatrices.push_back(Matrix4d::Identity());
    }
}

WhirlyKit::Point2f MapView::unwrapCoordinate(const WhirlyKit::Point2f &inPt) const
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

double MapView::heightAboveSurface() const
{
    return loc.z();
}

double MapView::minHeightAboveSurface() const
{
    if (continuousZoom)
        return absoluteMinHeight;
    else
        return 1.01 * getNearPlane();
}

double MapView::maxHeightAboveSurface() const
{
    return defaultFarPlane - 1.0;
}

void MapView::setLoc(WhirlyKit::Point3d newLoc)
{
    setLoc(newLoc,true);
}

void MapView::setLoc(const WhirlyKit::Point3d &newLoc,bool runUpdates)
{
    loc = newLoc;
    
    // If we get down below the inflection point we'll start messing
    //  with the field of view.  Not ideal, but simple.
    if (continuousZoom)
    {
        if (loc.z() < heightInflection)
        {
            const double t = 1.0 - (heightInflection - loc.z()) / (heightInflection - absoluteMinHeight);
            setPlanes(t * (defaultNearPlane-absoluteMinNearPlane) + absoluteMinNearPlane,
                      loc.z() + getNearPlane());
        } else {
            setPlanes(defaultNearPlane, defaultFarPlane);
        }
        updateParams();
    }

    if (runUpdates)
        runViewUpdates();
}

void MapView::setRotAngle(double newRotAngle,bool runUpdates)
{
    rotAngle = newRotAngle;
    runViewUpdates();
    if (runUpdates)
        runViewUpdates();
}

Eigen::Matrix4d MapView::calcFullMatrix() const
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
    const Point3d worldLoc(inWorldLoc.x(),inWorldLoc.y(),inWorldLoc.z());
    
    // Run the model point through the model transform (presumably what they passed in)
    const Eigen::Matrix4d modelTrans = *transform;
    const Matrix4d modelMat = modelTrans;
    const Vector4d screenPt = modelMat * Vector4d(worldLoc.x(),worldLoc.y(),worldLoc.z(),1.0);

    // Intersection with near gives us the same plane as the screen 
    Point3d ray = Point3d(screenPt.x(), screenPt.y(), screenPt.z()) / screenPt.w();
    ray *= -getNearPlane()/ray.z();

    // Now we need to scale that to the frame
    Point2d ll,ur;
    double near,far;
    calcFrustumWidth(frameSize.x(),frameSize.y(),ll,ur,near,far);
    const double u = (ray.x() - ll.x()) / (ur.x() - ll.x());
    const double v = (ray.y() - ll.y()) / (ur.y() - ll.y());

    return {u * frameSize.x(), (1 - v) * frameSize.y()};
}

Eigen::Vector3d MapView::eyePos() const
{
    Eigen::Matrix4d modelMat = calcModelMatrix().inverse();
    
    Vector4d newUp = modelMat * Vector4d(0,0,1,1);
    return Vector3d(newUp.x(),newUp.y(),newUp.z());
}

/// Set the change delegate
void MapView::setDelegate(MapViewAnimationDelegateRef inDelegate)
{
    delegate = std::move(inDelegate);
}
    
MapViewAnimationDelegateRef MapView::getDelegate()
{
    return delegate;
}

/// Called to cancel a running animation
void MapView::cancelAnimation()
{
    delegate.reset();
}

/// Renderer calls this every update.
void MapView::animate()
{
    // Have to hold on to the delegate because it can call cancelAnimation.... which frees the delegate
    if (auto theDelegate = delegate)
    {
        theDelegate->updateView(this);
    }
}
    
ViewStateRef MapView::makeViewState(SceneRenderer *renderer)
{
    return std::make_shared<MapViewState>(this,renderer);
}

MapViewState::MapViewState(MapView *mapView,SceneRenderer *renderer)
: ViewState(mapView,renderer)
{
    heightAboveSurface = mapView->getLoc().z();
}

bool MapViewState::pointOnPlaneFromScreen(const WhirlyKit::Point2f &pt,const Eigen::Matrix4d &modelTrans,const WhirlyKit::Point2f &frameSize, WhirlyKit::Point3d &hit, bool clip)
{
    // Back Project the screen point into model space
    Point3d screenPt = pointUnproject(Point2d(pt.x(),pt.y()), frameSize.x(), frameSize.y(), clip);
    
    // Run the screen point and the eye point (origin) back through
    //  the model matrix to get a direction and origin in model space
    Eigen::Matrix4d invModelMat = modelTrans.inverse();
    Point3d eyePt(0,0,0);
    Eigen::Vector4d modelEye = invModelMat * Eigen::Vector4d(eyePt.x(),eyePt.y(),eyePt.z(),1.0);
    Eigen::Vector4d modelScreenPt = invModelMat * Eigen::Vector4d(screenPt.x(),screenPt.y(),screenPt.z(),1.0);
    
    // Now intersect with the plane at (0,0)
    // Okay, this is kind of overkill
    Eigen::Vector4d dir4 = modelScreenPt - modelEye;
    Eigen::Vector3d dir(dir4.x(),dir4.y(),dir4.z());
    
    if (dir.z() == 0.0)
        return false;
    dir.normalize();
    double t = - modelEye.z() / dir.z();
    hit = Point3d(modelEye.x(),modelEye.y(),modelEye.z()) + dir * t;
    
    return true;
}

}
