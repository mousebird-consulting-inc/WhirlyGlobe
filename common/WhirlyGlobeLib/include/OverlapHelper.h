/*
 *  OverlapHelper.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/28/15.
 *  Copyright 2011-2022 mousebird consulting.
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

#import <math.h>
#import <set>
#import <map>
#import "Identifiable.h"
#import "BasicDrawable.h"
#import "Scene.h"
#import "SceneRenderer.h"
#import "ScreenSpaceBuilder.h"
#import "SelectionManager.h"
#import "WhirlyVector.h"


namespace WhirlyKit
{
struct LayoutObjectEntry;
class LayoutObject;
using LayoutObjectEntryRef = std::shared_ptr<LayoutObjectEntry>;

// We use this to avoid overlapping labels
struct OverlapHelper
{
    OverlapHelper(const Mbr &mbr,int sizeX,int sizeY,size_t totalObjs);

    // Try to add an object.  Might fail (kind of the whole point).
    bool addCheckObject(const Point2dVector &pts, const char* mergeID = nullptr);
    bool addCheckObject(const Point2dVector &pts, const std::string &mergeID);

    // See if there's an object in the way
    bool checkObject(const Point2dVector &pts, const char* mergeID = nullptr);
    bool checkObject(const Point2dVector &pts, const std::string &mergeID);

    // Force an object in no matter what
    void addObject(Point2dVector pts, std::string mergeID = std::string());
    
protected:
    void calcCells(const Mbr &objMbr, int &sx, int &sy, int &ex, int &ey);
    bool checkObject(const Point2dVector &pts, const Mbr &objMbr,
                     int sx, int sy, int ex, int ey,
                     const char* mergeID);
    void addObject(Point2dVector pts, std::string mergeID,
                   int sx, int sy, int ex, int ey);

    struct GridCell
    {
        // Indexes into objects vector
        std::vector<int> objIndexes;
    };

    // Object and its bounds
    struct BoundedObject
    {
        BoundedObject() = default;
        BoundedObject(const BoundedObject&) = default;
        BoundedObject& operator=(const BoundedObject&) = default;

        BoundedObject(const Point2dVector& inPts, const char* id) :
                pts(inPts), mergeID(id ? id : std::string())
        {
        }

        BoundedObject(Point2dVector&& inPts, std::string &&id) :
                pts(std::move(inPts)), mergeID(std::move(id))
        {
        }

        BoundedObject(BoundedObject&& that) :
                pts(std::move(that.pts)), mergeID(std::move(that.mergeID))
        {
        }
        BoundedObject& operator=(BoundedObject&& that)
        {
            if (this != &that)
            {
                pts = std::move(that.pts);
                mergeID = std::move(that.mergeID);
            }
            return *this;
        }

        Point2dVector pts;
        std::string mergeID;
    };

    GridCell &cellAt(int x, int y) { return grid[y * sizeX + x]; }

    Mbr mbr;
    int sizeX;
    int sizeY;
    size_t totalObjs;
    Point2f cellSize;
    std::vector<BoundedObject> objects;
    std::vector<GridCell> grid;

    // Estimate the fraction of objects likely to fall in a given cell
    const double overlapHeuristic = 0.1;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
};

// Used to figure out what clusters
class ClusterHelper
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    ClusterHelper(const Mbr &mbr,int sizeX, int sizeY, float resScale, Point2d clusterMarkerSize);
    
    // Add an object, possibly forming a group
    void addObject(LayoutObjectEntryRef objEntry,const Point2dVector &pts);

    // Deal with cluster to cluster overlap
    void resolveClusters(volatile bool &cancel);
    
    // Single object with its bounds
    struct ObjectWithBounds
    {
        Point2dVector pts;
        Point2d center;
    };
    
    // Simple object we're trying to cluster
    struct SimpleObject : public ObjectWithBounds
    {
        SimpleObject() = default;
        std::shared_ptr<LayoutObjectEntry> objEntry;
        int parentObject = -1;
    };
    
    // Object we create when there are overlaps
    struct ClusterObject : public ObjectWithBounds
    {
        std::vector<int> children;
    };
    
    // List of objects for this cluster
    void objectsForCluster(const ClusterObject &cluster,
                           std::vector<LayoutObjectEntryRef> &layoutObjs);

    // Add the given index to the cells it covers
    void addToCells(const Mbr &objMbr, int index);
    
    // Remove the given index from the cells it covers
    void removeFromCells(const Mbr &objMbr, int index);
    
    // Return all the objects within the overlap
    void findObjectsWithin(const Mbr &mbr,std::set<int> &objSet);
    
    void calcCells(const Mbr &mbr,int &sx,int &sy,int &ex,int &ey);

    Point2d clusterMarkerSize;
    
    Mbr mbr;
    std::vector<SimpleObject> simpleObjects;
    std::vector<ClusterObject> clusterObjects;

    // Grid we're sorting into for fast lookup
    int sizeX,sizeY;
    float resScale;
    Point2d cellSize;
    std::vector<std::set<int> > grid;
};
    
}
