/*
 *  QuadTracker.java
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
package com.mousebird.maply;

/**
 * The quad tracker keeps track of quad tree nodes.
 * <p>
 * This object tracks quad tree nodes as they're added to and removed from an internal quad tree
 * that tracks them by screen importance.
 */
public class QuadTracker
{
    // Required by the JNI side
    private QuadTracker()
    {
    }

    /**
     * Set up the quad tracker with a valid globe controller.
     */
    public QuadTracker(GlobeController globeController,CoordSystem coordSystem,Point2d ll,Point2d ur,int minLevel)
    {
        initialise(globeController.globeView,globeController.renderWrapper.maplyRender,globeController.coordAdapter,coordSystem,ll,ur,minLevel);
    }

    public void finalize(){
        dispose();
    }

    /**
     * Run the queries against the samples in the QuadTrackerPointReturn.
     * Each sample should have a screen location set.  The rest will be filled in by
     * the query.
     */
    public void queryTiles(QuadTrackerPointReturn ptReturn)
    {
        queryTilesNative(ptReturn.numPts,ptReturn.screenLocs,ptReturn.tileIDs,ptReturn.coordLocs,ptReturn.tileLocs);
    }

    native void queryTilesNative(int numPts,double[] screenLocs,int[] tileIDs,double coordLocs[],double tileLocs[]);

    /**
     * Add a tile to be tracked.
     */
    public void addTile(MaplyTileID tileID)
    {
        addTile(tileID.x,tileID.y,tileID.level);
    }

    native void addTile(int x, int y, int level);

    /**
     * Remove the given tile from tracking.
     */
    public void removeTile(MaplyTileID tileID)
    {
        removeTile(tileID.x,tileID.y,tileID.level);
    }

    native void removeTile(int x, int y, int level);

    /**
     * Return the minimum level.  The maximum isn't set.
     */
    public native int getMinLevel();

    static {
        nativeInit();
    }

    private static native void nativeInit();
    native void initialise(GlobeView globeView,MaplyRenderer renderer,CoordSystemDisplayAdapter adapter,CoordSystem coordSystem,Point2d ll,Point2d ur,int minLevel);
    native void dispose();
    private long nativeHandle;
}
