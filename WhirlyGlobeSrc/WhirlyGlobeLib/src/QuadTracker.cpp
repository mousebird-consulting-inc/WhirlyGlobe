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

    QuadTrackerPointReturn::QuadTrackerPointReturn()
    {
    }
    
    void QuadTrackerPointReturn::setScreenU(double _screenU)
    {
        screenU = _screenU;
    }
    
    double QuadTrackerPointReturn::getScreenU()
    {
        return screenU;
    }
    
    void QuadTrackerPointReturn::setScreenV(double _screenV)
    {
        screenV = _screenV;
    }
    
    double QuadTrackerPointReturn::getScreenV()
    {
        return screenV;
    }
    
    void QuadTrackerPointReturn::setMaplyTileID(MaplyTileID _maplyTileID)
    {
        tileID = _maplyTileID;
    }
    
    MaplyTileID QuadTrackerPointReturn::getMaplyTileID()
    {
        return tileID;
    }
    
    void QuadTrackerPointReturn::setPadding(int _padding)
    {
        padding = _padding;
    }
    
    int QuadTrackerPointReturn::getPadding()
    {
        return padding;
    }
    
    void QuadTrackerPointReturn::setLocX(double _locX)
    {
        locX = _locX;
    }
    
    double QuadTrackerPointReturn::getLocX()
    {
        return locX;
    }
    
    void QuadTrackerPointReturn::setLocY(double _locY)
    {
        locY = _locY;
    }
    
    double QuadTrackerPointReturn::getLocY()
    {
        return locY;
    }
    
    void QuadTrackerPointReturn::setTileU(double _tileU)
    {
        tileU = _tileU;
    }
    
    double QuadTrackerPointReturn::getTileU()
    {
        return tileU;
    }
    
    void QuadTrackerPointReturn::setTileV(double _tileV)
    {
        tileV = _tileV;
    }
    
    double QuadTrackerPointReturn::getTileV()
    {
        return tileV;
    }
    
    
    TileWrapper::TileWrapper()
    {
    }
    
    TileWrapper::TileWrapper(MaplyTileID _tileID)
    {
        tileID = _tileID;
    }
    
    QuadTracker::QuadTracker(WhirlyGlobe::GlobeView *viewC)
    {
        theView = viewC;
        pthread_mutex_init(&tilesLock, NULL);

    }
    
    QuadTracker::~QuadTracker()
    {
        pthread_mutex_destroy(&tilesLock);
    }
    
    void QuadTracker::addTile(MaplyTileID tileID)
    {
        pthread_mutex_lock(&tilesLock);
        
        TileWrapper tile(tileID);
        if (tileSet.find(tile) == tileSet.end())
            tileSet.insert(tile);
        
        pthread_mutex_unlock(&tilesLock);
    }
    
    void QuadTracker::removeTile(MaplyTileID tileID)
    {
        pthread_mutex_lock(&tilesLock);
        
        TileWrapper tile(tileID);
        auto it = tileSet.find(tile);
        if (it != tileSet.end())
            tileSet.erase(it);
        
        pthread_mutex_unlock(&tilesLock);
    }
    
    void QuadTracker::tiles(WhirlyKit::QuadTrackerPointReturn *tilesInfo, int numPts)
    {
        if (!coordSys)
            return;
        
        WHIRLYKIT_LOGV("Tiles start");

        Point2d frameSize(renderer->framebufferWidth, renderer->framebufferHeight);

        pthread_mutex_lock(&tilesLock);

        Point3d hit;

        Eigen::Matrix4d modelTrans = theView->calcFullMatrix();
        Eigen::Matrix4d invModelMat = modelTrans.inverse();
        Point3d eyePt(0,0,0);
        Eigen::Vector4d modelEye = invModelMat * Eigen::Vector4d(eyePt.x(), eyePt.y(), eyePt.z(), 1.0);

        double mbrSpanX = ur.x() - ll.x();
        double mbrSpanY = ur.y() - ll.y();
        
        Point2d ll_int, ur_int;
        double near, far;
        
        theView->calcFrustumWidth(frameSize.x(), frameSize.y(), ll_int, ur_int, near, far);
        
        //Work through the points to test
        for (unsigned int ii= 0; ii< numPts; ii++) {
            
            QuadTrackerPointReturn *trackInfo = &tilesInfo[ii];
            
            double u = trackInfo->getScreenU();
            double v = 1.0 - trackInfo->getScreenV();
            
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
                Point3d localPt = theView->coordAdapter->displayToLocal(hit);
                //This point is in local (tile) system
                Point3d coordPt = CoordSystemConvert3d(theView->coordAdapter->getCoordSystem(), coordSys, localPt);
                
                trackInfo->setLocX(coordPt.x());
                trackInfo->setLocY(coordPt.y());
                
                //Clip to the overal bounds
                trackInfo->setTileU((coordPt.x()-ll_int.x())/mbrSpanX);
                trackInfo->setTileV((coordPt.y()-ll_int.y())/mbrSpanY);
                trackInfo->setTileU(std::min(trackInfo->getTileU(), 1.0));
                trackInfo->setTileU(std::max(trackInfo->getTileU(), 0.0));
                trackInfo->setTileV(std::min(trackInfo->getTileV(), 1.0));
                trackInfo->setTileV(std::max(trackInfo->getTileV(), 0.0));
                
                //Dive down looking for the highest resolution tile avaliable
                
                MaplyTileID tileID = trackInfo->getMaplyTileID();
                tileID.level = 0;
                tileID.x = 0;
                tileID.y = 0;
                trackInfo->setMaplyTileID(tileID);
                for (;;) {
                    MaplyTileID nextTile;
                    nextTile.level = trackInfo->getMaplyTileID().level+1;
                    int childY = (trackInfo->getTileU() < 0.5) ? 0 : 1;
                    int childX = (trackInfo->getTileV() < 0.5) ? 0 : 1;
                    nextTile.x = 2 * trackInfo->getMaplyTileID().x + childX;
                    nextTile.y = 2 * trackInfo->getMaplyTileID().y + childY;
                    
                    //See if this tile is here
                    
                    TileWrapper testTile(nextTile);
                    if (trackInfo->getMaplyTileID().level >= minLevel && tileSet.find(testTile) == tileSet.end())
                        break;
                    trackInfo->setMaplyTileID(nextTile);
                    trackInfo->setTileU(2.0*trackInfo->getTileU() - childX*1.0);
                    trackInfo->setTileV(2.0*trackInfo->getTileV() - childY*1.0);
                }
            } else {
                MaplyTileID tileID = trackInfo->getMaplyTileID();
                tileID.level = -1;
                trackInfo->setMaplyTileID(tileID);
            }
        }
        WHIRLYKIT_LOGV("tiles finished");
        for (unsigned int ii= 0; ii< numPts; ii++) {
            
            QuadTrackerPointReturn *trackInfo = &tilesInfo[ii];
            WHIRLYKIT_LOGV("Value %i: X: %d Y: %d Level: %d", ii, trackInfo->getMaplyTileID().x, trackInfo->getMaplyTileID().y, trackInfo->getMaplyTileID().level);
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
    
    void QuadTracker::setAdapter(WhirlyKit::CoordSystemDisplayAdapter *adapter)
    {
        coordAdapter = adapter;
        
    }
    
    void QuadTracker::setRenderer(WhirlyKit::SceneRendererES *_renderer)
    {
        renderer = _renderer;
    }
    
}

