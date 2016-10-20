//
//  IntersectionManager.cpp
//  WhirlyGlobeLib
//
//  Created by Steve Gifford on 6/29/16.
//
//

#include "IntersectionManager.h"

using namespace Eigen;

namespace WhirlyKit
{

IntersectionManager::IntersectionManager(Scene *scene)
: scene(scene)
{
    pthread_mutex_init(&mutex, NULL);
}
    
IntersectionManager::~IntersectionManager()
{
    pthread_mutex_destroy(&mutex);
}
    
void IntersectionManager::addIntersectable(Intersectable *intersect)
{
    intersectables.insert(intersect);
}

/// Remove an intersectable object
void IntersectionManager::removeIntersectable(Intersectable *intersect)
{
    intersectables.erase(intersect);
}

/// Look for the nearest intersection and return the point (in display coordinates)
bool IntersectionManager::findIntersection(SceneRendererES *renderer,View *view,const Point2f &frameSize,const Point2f &touchPt,Point3d &iPt,double &dist)
{
    Point3d minPt;
    double minDist = std::numeric_limits<double>::max();

    Eigen::Matrix4d fullMat = view->calcFullMatrix();
    Matrix4d invFullMat = fullMat.inverse();

    // Back project the point from screen space into model space
    Point3d tapPt = view->pointUnproject(touchPt,frameSize.x(),frameSize.y(),true);

    // Run the screen point and the eye point (origin) back through
    //  the model matrix to get a direction and origin in model space
    Point3d eyePt(0,0,0);
    Vector4d modelEye = invFullMat * Vector4d(0.0,0.0,0.0,1.0);
    Vector4d modelScreenPt = invFullMat * Vector4d(tapPt.x(),tapPt.y(),tapPt.z(),1.0);
    
    double eyeLen = modelEye.norm();
    double screenLen = modelScreenPt.norm();
    
    Vector4d dir4 = modelScreenPt - modelEye;
    Vector3d org(modelEye.x(),modelEye.y(),modelEye.z());
    Vector3d dir(dir4.x(),dir4.y(),dir4.z());
    dir.normalize();
    
    pthread_mutex_lock(&mutex);
    
    for (auto inter : intersectables)
    {
        Point3d thisPt;
        double thisDist;
        if (inter->findClosestIntersection(renderer, view, frameSize, touchPt, org, dir, thisPt, thisDist))
        {
            if (thisDist < minDist)
            {
                minDist = thisDist;
                minPt = thisPt;
            }
        }
    }
    
    pthread_mutex_unlock(&mutex);
    
    if (minDist != std::numeric_limits<double>::max())
    {
        iPt = minPt;
        return true;
    }
    
    return false;
}

}
