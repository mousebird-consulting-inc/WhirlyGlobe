/*
 *  MaplyLayerViewWatcher.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/14/12.
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

#import "MaplyViewState.h"

using namespace WhirlyKit;

namespace Maply
{

MapViewState::MapViewState(MapView *mapView,WhirlyKit::SceneRendererES *renderer)
    : ViewState(mapView,renderer)
{
    heightAboveSurface = mapView->getLoc().z();
}

bool MapViewState::pointOnPlaneFromScreen(WhirlyKit::Point2d pt, Eigen::Matrix4d transform, WhirlyKit::Point2f frameSize, WhirlyKit::Point3d hit, bool clip)
{
    // Back Project the screen point into model space
    Point3d screenPt = pointUnproject(pt, frameSize.x(), frameSize.y(), clip);

    // Run the screen point and the eye point (origin) back through
    //  the model matrix to get a direction and origin in model space
    Eigen::Matrix4d modelTrans = transform;
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
