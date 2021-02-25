/*
 *  VectorTileData.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/12/19.
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

import android.widget.CompoundButton;

import java.lang.reflect.Array;
import java.util.ArrayList;
import java.util.Vector;

/**
 * When parsing a tile of vector data, we need to pass input and output data around.
 * This encapulates the extent information and output data.  It's passed to the individual
 * styles which do the work.
 */
public class VectorTileData
{
    public VectorTileData() { initialise(); }

    // Initialize with a specific tile and bounds
    public VectorTileData(TileID tileID,Mbr bounds,Mbr geoBounds)
    {
        initialise(tileID.x,tileID.y,tileID.level,bounds.ll,bounds.ur,geoBounds.ll,geoBounds.ur);
    }

    public void finalize()
    {
        dispose();
    }

    /**
     * Tile ID for this tile
     */
    public TileID getTileID() {
        int[] info = getTileIDNative();
        TileID ident = new TileID();
        ident.x = info[0];  ident.y = info[1];  ident.level = info[2];

        return ident;
    }

    private native int[] getTileIDNative();

    /**
     * Bounding box in local coordinates.
     */
    public Mbr getBounds()
    {
        Mbr mbr = new Mbr();
        mbr.initialize();
        getBoundsNative(mbr.ll,mbr.ur);

        return mbr;
    }

    private native void getBoundsNative(Point2d ll,Point2d ur);

    /**
     * Bounding box in geographic.
     */
    public Mbr getGeoBounds()
    {
        Mbr mbr = new Mbr();
        mbr.initialize();
        getBoundsNative(mbr.ll,mbr.ur);

        return mbr;
    }

    private native void getGeoBoundsNative(Point2d ll,Point2d ur);

    /**
     * Return the component objects created for this tile.
     */
    public native ComponentObject[] getComponentObjects();

    /**
     * Return the component objects associated with the given category, if any.
     */
    public native ComponentObject[] getComponentObjects(String category);

    /**
     * Add a single component object to the tile we've parsed.
     */
    public native void addComponentObject(ComponentObject compObj);

    /**
     * Add a list of component objects to the tile we've parsed.
     */
    public native void addComponentObjects(ComponentObject[] compObjs);

    /**
     * Return any changes put into the change set rather than ComponentObjects.
     */
    public native ChangeSet getChangeSet();

    /**
     * Add a list of component objects to the tile we've parsed.
     */
    public void addComponentObjects(ArrayList<ComponentObject> compObjs) {
        addComponentObjects(compObjs.toArray(new ComponentObject[0]));
    }

    /**
     * Return all the vector objects parsed for this tile, assuming they were set
     * to be preserved.
     */
    public native VectorObject[] getVectors();

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void initialise(int x,int y,int level,Point2d boundsLL,Point2d boundsUR,Point2d geoBoundsLL,Point2d geoBoundsUR);
    native void dispose();

    private long nativeHandle;
}
