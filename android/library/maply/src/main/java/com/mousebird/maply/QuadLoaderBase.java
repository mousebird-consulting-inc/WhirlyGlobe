/*
 *  QuadLoaderBase.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/22/19.
 *  Copyright 2011-2019 mousebird consulting
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

import java.lang.ref.WeakReference;

/**
 * Base class for the quad loaders.
 * <br>
 * The image, frame, and data paging loaders all share much of the same functionality.
 */
public class QuadLoaderBase
{
    protected QuadLoaderBase(BaseController inControl)
    {
        control = new WeakReference<BaseController>(inControl);
    }

    /**
     *  Control how tiles are indexed, either from the lower left or the upper left.
     *
     *  If set, we'll use the OSM approach (also Google Maps) to y indexing.  That's that default and it's normally what you're run into.
     *
     *  Strictly speaking, TMS addressing (the standard) is flipped the other way.  So if your tile source looks odd, try setting this to false.
     *
     *  Default value is true.
     */
    public native void setFlipY(boolean newVal);

    /**
     * Set for a lot of debugging output.
     */
    public native void setDebugMode(boolean debugMode);

    private WeakReference<BaseController> control;

    /**
     * Controller associated with this quad loader.
     * This is where you send geometry and such.
     */
    BaseController getController()
    {
        return control.get();
    }

    /**
     *  Calculate the bounding box for a single tile in geographic.
     *  <br>
     *  This is a utility method for calculating the extents of a given tile in geographic (e.g. lon/lat).
     *
     *  @param tileID The ID for the tile we're interested in.
     *
     *  @return The lower left and upper right corner of the tile in geographic coordinates.
     *          Returns null in case of error.
     */
    Mbr geoBoundsForTile(TileID tileID)
    {
        Mbr mbr = new Mbr();
        geoBoundsForTileNative(tileID.x,tileID.y,tileID.level,mbr.ll,mbr.ur);

        return mbr;
    }

    protected native void geoBoundsForTileNative(int x,int y,int level,Point2d ll,Point2d ur);

    /**
     *  Calculate the bounding box for a single tile in the local coordinate system.
     *
     *  This utility method calculates the bounding box for a tile in the coordinate system used for the layer.
     *
     *  @param tileID The ID for the tile we're interested in.
     *
     *  @return The lower left and upper right corner of the tile in local coordinates.
     */
    Mbr boundsForTile(TileID tileID)
    {
        Mbr mbr = new Mbr();
        boundsForTileNative(tileID.x,tileID.y,tileID.level,mbr.ll,mbr.ur);

        return mbr;
    }

    protected native void boundsForTileNative(int x,int y,int level,Point2d ll,Point2d ur);

    /**
     *  Return the center of the tile in display coordinates.
     *
     *  @param tileID The ID for the tile we're interested in.
     *
     *  @return Return the center in display space for the given tile.
     */
    Point3d displayCenterForTile(TileID tileID)
    {
        Point3d pt = new Point3d();
        displayCenterForTileNative(tileID.x,tileID.y,tileID.level,pt);
        return pt;
    }

    protected native void displayCenterForTileNative(int x,int y,int level,Point3d pt);

    protected TileFetcher tileFetcher = null;

    /**
     * Use a specific tile fetcher rather than the one shared by everyone else.
     */
    void setTileFetcher(TileFetcher newFetcher) {
        tileFetcher = newFetcher;
    }

    protected LoaderInterpreter loadInterp = null;

    /**
     * Set the interpreter for the data coming back.  If you're just getting images, don't set this.
     */
    void setLoaderInterpreter(LoaderInterpreter newInterp) {
        loadInterp = newInterp;
    }

    /**
     * Turn off the loader and shut things down.
     * <br>
     * This unregisters us with the sampling layer and shuts down the various objects we created.
     */
    public void shutdown()
    {
        tileFetcher = null;
        loadInterp = null;

        // TODO: Shut down sampling layer and call this on layer thread
        ChangeSet changes = new ChangeSet();
        cleanupNative(changes);
        // TODO: Submit the changes before shutdown
    }

    protected native void cleanupNative(ChangeSet changes);

    // Used to initialize the loader for certain types of data.
    enum Mode {SingleFrame,MultiFrame,Object};

    public void finalize()
    {
        dispose();
    }
    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise(SamplingParams params,int numFrames,int mode);
    native void dispose();
    private long nativeHandle;
}
