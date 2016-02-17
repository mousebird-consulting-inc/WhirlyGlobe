/*
 *  QuadTracker.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
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
#include "QuadTracker.h"
#include "WhirlyGlobe.h"
#import <set>


namespace WhirlyKit
{

QuadTrackerPointReturn::QuadTrackerPointReturn(int numPts,double *screenLocs,int *tileIDs,double *locs,double *tileLocs)
    : numPts(numPts), screenLocs(screenLocs), tileIDs(tileIDs), coordLocs(locs), tileLocs(tileLocs)
{
}

void QuadTrackerPointReturn::setScreenLoc(int which,double screenU,double screenV)
{
    if (which >= numPts)
        return;
    screenLocs[2*which] = screenU;
    screenLocs[2*which+1] = screenV;
}
    
void QuadTrackerPointReturn::getScreenLoc(int which,double &u,double &v)
{
    if (which >= numPts)
        return;
    u = screenLocs[2*which];
    v = screenLocs[2*which+1];
}
    
void QuadTrackerPointReturn::setTileID(int which,int x,int y,int level)
{
    if (which >= numPts)
        return;
    tileIDs[3*which] = x;
    tileIDs[3*which+1] = y;
    tileIDs[3*which+2] = level;
}
    
void QuadTrackerPointReturn::setCoordLoc(int which,double locX,double locY)
{
    if (which >= numPts)
        return;
    coordLocs[2*which] = locX;
    coordLocs[2*which+1] = locY;
}
    
void QuadTrackerPointReturn::setTileLoc(int which,double tileLocX,double tileLocY)
{
    if (which >= numPts)
        return;
    tileLocs[2*which] = tileLocX;
    tileLocs[2*which+1] = tileLocY;
}


TileWrapper::TileWrapper()
{
}

TileWrapper::TileWrapper(const Quadtree::Identifier &tileID)
    : tileID(tileID)
{
}

QuadTracker::QuadTracker(WhirlyGlobe::GlobeView *globeView,SceneRendererES *renderer,CoordSystemDisplayAdapter *adapter)
    : globeView(globeView), renderer(renderer), coordAdapter(adapter)
{
    pthread_mutex_init(&tilesLock, NULL);

}

QuadTracker::~QuadTracker()
{
    pthread_mutex_destroy(&tilesLock);
}

void QuadTracker::addTile(const Quadtree::Identifier &tileID)
{
    pthread_mutex_lock(&tilesLock);
    
    TileWrapper tile(tileID);
    if (tileSet.find(tile) == tileSet.end())
        tileSet.insert(tile);
    
    pthread_mutex_unlock(&tilesLock);
}

void QuadTracker::removeTile(const Quadtree::Identifier &tileID)
{
    pthread_mutex_lock(&tilesLock);
    
    TileWrapper tile(tileID);
    auto it = tileSet.find(tile);
    if (it != tileSet.end())
        tileSet.erase(it);
    
    pthread_mutex_unlock(&tilesLock);
}

void QuadTracker::tiles(WhirlyKit::QuadTrackerPointReturn *trackInfo, int numPts)
{
    if (!coordSys)
        return;
    
    WHIRLYKIT_LOGV("Tiles start");

    Point2d frameSize(renderer->framebufferWidth, renderer->framebufferHeight);

    pthread_mutex_lock(&tilesLock);

    Point3d hit;

    Eigen::Matrix4d modelTrans = globeView->calcFullMatrix();
    Eigen::Matrix4d invModelMat = modelTrans.inverse();
    Point3d eyePt(0,0,0);
    Eigen::Vector4d modelEye = invModelMat * Eigen::Vector4d(eyePt.x(), eyePt.y(), eyePt.z(), 1.0);

    double mbrSpanX = ur.x() - ll.x();
    double mbrSpanY = ur.y() - ll.y();
    
    Point2d ll_int, ur_int;
    double near, far;
    
    globeView->calcFrustumWidth(frameSize.x(), frameSize.y(), ll_int, ur_int, near, far);
    
    //Work through the points to test
    for (unsigned int ii= 0; ii< numPts; ii++) {
        double u,v;
        trackInfo->getScreenLoc(ii,u,v);
        v = 1.0 - v;
        
        //Now come up with a point in 3 space between ll and ur
        
        Point3d viewPlaneLoc(u *(ur_int.x()-ll_int.x()) + ll_int.x(), v * (ur_int.y() - ll_int.y()) + ll_int.y(), -near);
        
        //Run the screen point and the eye point (origin) back through the model matrixto get a direction and origin in model space
        Eigen::Vector4d modelScreenPt = invModelMat * Eigen::Vector4d(viewPlaneLoc.x(), viewPlaneLoc.y(), viewPlaneLoc.z(), 1.0);
        
        //Now intersect the with a unit sphere to see where we hit
        
        Eigen::Vector4d dir4 = modelScreenPt - modelEye;
        Eigen::Vector3d dir(dir4.x(), dir4.y(), dir4.z());
        Point3d hit;
        double t;
        
        if (IntersectUnitSphere(Eigen::Vector3d(modelEye.x(), modelEye.y(), modelEye.z()), dir, hit, &t) && t>0.0) {
            Point3d localPt = globeView->coordAdapter->displayToLocal(hit);
            //This point is in local (tile) system
            Point3d coordPt = CoordSystemConvert3d(globeView->coordAdapter->getCoordSystem(), coordSys, localPt);
            
            trackInfo->setCoordLoc(ii, coordPt.x(), coordPt.y());
            
            //Clip to the overal bounds
            double tileU = (coordPt.x()-ll_int.x())/mbrSpanX;
            double tileV = (coordPt.y()-ll_int.y())/mbrSpanY;
            tileU = std::max(std::min(tileU, 1.0),0.0);
            tileV = std::max(std::min(tileV, 1.0),0.0);
            trackInfo->setTileLoc(ii, tileU, tileV);
            
            //Dive down looking for the highest resolution tile avaliable
            Quadtree::Identifier tileID;
            tileID.level = 0;
            tileID.x = 0;
            tileID.y = 0;
            for (;;) {
                Quadtree::Identifier nextTile;
                nextTile.level = tileID.level+1;
                int childY = (tileU < 0.5) ? 0 : 1;
                int childX = (tileV < 0.5) ? 0 : 1;
                nextTile.x = 2 * tileID.x + childX;
                nextTile.y = 2 * tileID.y + childY;
                
                //See if this tile is here
                TileWrapper testTile(nextTile);
                if (tileID.level >= minLevel && tileSet.find(testTile) == tileSet.end())
                    break;
                tileID = nextTile;
                tileU = 2.0*tileU - childX*1.0;
                tileV = 2.0*tileV - childY*1.0;
            }
            trackInfo->setTileID(ii,tileID.x,tileID.y,tileID.level);
        } else {
            Quadtree::Identifier tileID;
            tileID.x = 0; tileID.y = 0;  tileID.level = -1;
            trackInfo->setTileID(ii,tileID.x,tileID.y,tileID.level);
        }
    }
    WHIRLYKIT_LOGV("tiles finished");
    for (unsigned int ii= 0; ii< numPts; ii++) {
        
//        QuadTrackerPointReturn *trackInfo = &tilesInfo[ii];
//        WHIRLYKIT_LOGV("Value %i: X: %d Y: %d Level: %d", ii, trackInfo->getMaplyTileID().x, trackInfo->getMaplyTileID().y, trackInfo->getMaplyTileID().level);
    }
    
    pthread_mutex_unlock(&tilesLock);
}

void QuadTracker::setCoordSys(WhirlyKit::CoordSystem *_coordSys, Point2d _ll, Point2d _ur)
{
    coordSys = _coordSys;
    ll = _ll;
    ur = _ur;
}

void QuadTracker::setMinLevel(int _minLevel)
{
    minLevel = _minLevel;
}

int QuadTracker::getMinLevel()
{
    return minLevel;
}
    
}

