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
#import <set>
#import <Eigen/Eigen>
#import "Quadtree.h"
#import "GlobeView.h"
#import "SceneRendererES.h"

using namespace Eigen;

namespace WhirlyKit
{
    
/** @brief Return data for one or more point queries.
    @details You'll pass in an array of these to tiles:forScreenPts:numPts:
 */
class QuadTrackerPointReturn
{
public:
    /// @brief Construct with the data arrays we'll neeed
    /// @details Construct with the data arrays.  The class won't delete them.
    QuadTrackerPointReturn(int numPts,double *screenLocs,int *tileIDs,double *locs,double *tileLocs);
    
    /// @brief Location on screen scaled between (0,1)
    void setScreenLoc(int which,double screenU,double screenV);
    
    /// @brief Get the screen location for a sample
    void getScreenLoc(int which,double &u,double &v);

    /// @brief The tile the corresponding point belonged to.  Level set to -1 if invalid.
    void setTileID(int which,int x,int y,int level);
    
    /// @brief Location in coordinate system
    void setCoordLoc(int which,double locX,double locY);
    
    /// @brief Location within tile (scaled from 0-1)
    void setTileLoc(int which,double tileLocX,double tileLocY);
    
private:
    int numPts;
    
    // Screen location (U,V) - 2 doubles per
    double *screenLocs;

    // Tile IDs - 3 ints per
    int *tileIDs;

    // Location in coordinate system (X,Y) - 2 doubles per
    double *coordLocs;
    
    // Location within tile (X,Y) - 2 doubles per
    double *tileLocs;
};
    
/// Used in the quad tracker
class TileWrapper
{
public:
    TileWrapper();
    TileWrapper(const Quadtree::Identifier &tileID);
    
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
    Quadtree::Identifier tileID;
};
    
/** @brief The quad tracker keeps track of quad tree nodes.
    @details This object tracks quad tree nodes as they're added to and removed from an internal quad tree that tracks them by screen importance.*/
class QuadTracker
{
public:
    /** @brief Construct with a globe view.
      */
    QuadTracker(WhirlyGlobe::GlobeView *globeView,SceneRendererES *renderer,CoordSystemDisplayAdapter *adapter);
    ~QuadTracker();

    /** @brief Query the quad tracker for tiles and locations within them for a group of points.
     @details This is a bulk query for points within the tiles being tracked.
     @param tilesInfo This is both an input and output parameter.  Fill in the screenU and screenV values and you'll get back tileID and tileU and tileV.  tileID.level will be -1 if there was no hit for that point.
     @param numPts The number of points in the tilesInfo array.
     */
    void tiles(QuadTrackerPointReturn *tilesInfo, int numPts);
    
    /** @brief Add a tile to track.
     */
    void addTile(const Quadtree::Identifier &tileID);
    
    /** @brief Remove a tile from tracking
     */
    void removeTile(const Quadtree::Identifier &tileID);
    
    /** @brief Set the coordinate system and extents to cover.
      */
    void setCoordSys(CoordSystem *_coordSys, Point2d _ll, Point2d _ur);

    /** @brief Set the minimum zoom level
      */
    void setMinLevel(int _minLevel);

    /// @brief Return the min zoom level
    int getMinLevel();
    
private:
    CoordSystem *coordSys;
    pthread_mutex_t tilesLock;
    int minLevel;
    WhirlyGlobe::GlobeView *globeView;
    CoordSystemDisplayAdapter *coordAdapter;
    SceneRendererES *renderer;
    std::set<TileWrapper> tileSet;
    Point2d ll, ur;
};
    
}
