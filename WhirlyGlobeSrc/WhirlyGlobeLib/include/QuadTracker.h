/*
 *  QuadTracker.h
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
#import "WhirlyGlobe.h"
#import <set>
#import <Eigen/Eigen>

using namespace Eigen;

namespace WhirlyKit
{

struct MaplyTileID {
    int x, y, level;
};
    
struct MaplyBoundingBox {
    Point2f ll;
    Point2f ur;
};
    
//Return data for one or more points
class QuadTrackerPointReturn {
    
public:
    QuadTrackerPointReturn();
    
    void setScreenV(double _screenV);
    double getScreenV();
    void setScreenU(double _screenU);
    double getScreenU();
    void setMaplyTileID(MaplyTileID _maplyTileID);
    MaplyTileID getMaplyTileID();
    void setPadding(int _padding);
    int getPadding();
    void setLocX(double locX);
    double getLocX();
    void setLocY(double locY);
    double getLocY();
    void setTileU(double tileU);
    double getTileU();
    void setTileV(double tileV);
    double getTileV();
    
private:
    //Location on screen scaled between (0,1)
    double screenU, screenV;
    // The tile the corresponding point belonged to. Level set to -1 if invalid
    MaplyTileID tileID;
    
    //Required to make C/C++ bridge happy
    int padding;
    
    //Location in coordinate system
    double locX, locY;
    //Location within tile (scaled from 0-1)
    double tileU, tileV;
};
    
    
class TileWrapper {
  
public:
    TileWrapper();
    TileWrapper(MaplyTileID _tileID);
    
    bool operator < (const TileWrapper &that) const
    {
        if (tileID.level == that.tileID.level)
        {
            if (tileID.y == that.tileID.y){
                return  tileID.x < that.tileID.x;
            }
            return tileID.y < that.tileID.y;
        }
        return tileID.level < that.tileID.level;
    }
    MaplyTileID tileID;
};
   
    
/** @brief The quad tracker keeps track of quad tree nodes.
    @details This object tracks quad tree nodes as they're added to and removed from an internal quad tree that tracks them by screen importance.*/
    
class QuadTracker {
    
public:
    QuadTracker(WhirlyGlobe::GlobeView *viewC);
    ~QuadTracker();
    void tiles(QuadTrackerPointReturn * tilesInfo, int numPts);
    void addTile(MaplyTileID tileID);
    void removeTile(MaplyTileID tileID);
    
    void setCoordSys(CoordSystem *_coordSys, Point2d _ll, Point2d _ur);
    void setMinLevel(int _minLevel);
    int getMinLevel();
    void setAdapter(CoordSystemDisplayAdapter *adapter);
    void setRenderer(SceneRendererES *_renderer);
    
private:
    CoordSystem *coordSys;
    pthread_mutex_t tilesLock;
    int minLevel;
    WhirlyGlobe::GlobeView *theView;
    CoordSystemDisplayAdapter *coordAdapter;
    SceneRendererES *renderer;
    std::set<TileWrapper> tileSet;
    Point2d ll, ur;
};
    
}
