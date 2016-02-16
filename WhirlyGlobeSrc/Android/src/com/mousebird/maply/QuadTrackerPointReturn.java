/*
 *  QuadTrackerPointReturn.java
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
 * You pass in one of these to query where a whole mess of points fall
 * on a group of pages tiles.  This is useful for particle systems that
 * read from tiles (like wind tiles).
 */
public class QuadTrackerPointReturn
{
    public QuadTrackerPointReturn(int inNumPts)
    {
        numPts = inNumPts;
        screenLocs = new double[2*numPts];
        tileIDs = new int[3*numPts];
        coordLocs = new double[2*numPts];
        tileLocs = new double[2*numPts];
    }

    int numPts = 0;
    public double[] screenLocs;
    public int[] tileIDs;
    public double coordLocs[];
    public double tileLocs[];

    /**
     * Return the number of points.
     */
    public int getNumPoints() { return numPts; }

    /**
     * Set the location on the screen for the given sample.
     * @param which Which sample to set.
     * @param screenU X value on the screen
     * @param screenV Y value on the screen
     */
    public void setScreenLoc(int which,double screenU,double screenV)
    {
        screenLocs[2*which] = screenU;
        screenLocs[2*which+1] = screenV;
    }

    /**
     * Return the screen location (which you set earlier) for a given sample.
     */
    public Point2d getScreenLoc(int which)
    {
        Point2d screenLoc = new Point2d();
        screenLoc.setValue(screenLocs[2*which],screenLocs[2*which+1]);
        return screenLoc;
    }

    /**
     * Return the tileID where a given sample landed.
     * If level is set to -1, that means it didn't land.
     */
    public MaplyTileID getTileID(int which)
    {
        MaplyTileID tileID = new MaplyTileID();
        tileID.x = tileIDs[3*which];
        tileID.y = tileIDs[3*which+1];
        tileID.level = tileIDs[3*which+2];

        return tileID;
    }

    /**
     * Return the location in the local coordinate system.
     * If this is a globe, this is Spherical Mercator.
     */
    public Point2d getCoordLoc(int which)
    {
        Point2d coordLoc = new Point2d();
        coordLoc.setValue(coordLocs[2*which],coordLocs[2*which+1]);
        return coordLoc;
    }

    /**
     * Return the X coordinate in the local coordinate system for the sample.
     */
    public double getCoordLocX(int which)
    {
        return coordLocs[2*which];
    }

    /**
     * Return the Y coordinate in the local coordinate system for the sample.
     */
    public double getCoordLocY(int which)
    {
        return coordLocs[2*which+1];
    }

    /**
     * Return the location within a tile where the sample landed.
     * This is how you interpolate data values within a tile for the sample.
     */
    public Point2d getTileLoc(int which)
    {
        Point2d tileLoc = new Point2d();
        tileLoc.setValue(tileLocs[2*which],tileLocs[2*which+1]);
        return tileLoc;
    }

    /**
     * Return the X coordinate in the tile.
     */
    public double getTileLocU(int which)
    {
        return tileLocs[2*which];
    }

    /**
     * Return the Y coordinate in the tile.
     */
    public double getTileLocV(int which)
    {
        return tileLocs[2*which+1];
    }

}
