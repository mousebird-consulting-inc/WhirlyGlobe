/*
 *  SamplingParams.java
 *  WhirlyGlobeLib
 *
 *  Created by sjg
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

/**
 Sampling parameters.

 These are used to describe how we want to break down the globe or
 flat projection onto the globe.
 **/
public class SamplingParams
{
    public SamplingParams()
    { initialise(); }

    /**
     * Set the coordinate system for the model itself
     */
    public void setCoordSystem(CoordSystem coordSystem)
    {
        setCoordSystemNative(coordSystem,coordSystem.ll,coordSystem.ur);
    }

    protected native void setCoordSystemNative(CoordSystem coordSystem,Point3d ll,Point3d ur);

    /**
     * Return the coordinate system used for the sampling.
     */
    public native CoordSystem getCoordSystem();

    /**
     * Min zoom level for sampling.  Usually 0.
     */
    public native void setMinZoom(int minZoom);

    /**
     * Min zoom level for sampling.  Usually 0.
     */
    public native int getMinZoom();

    /**
     * Max zoom level for sampling.  Usually more than 0.
     */
    public native void setMaxZoom(int maxZoom);

    /**
     * Max zoom level for sampling.  Usually more than 0.
     */
    public native int getMaxZoom();

    /**
     * Maximum number of tiles to display at once.
     * The system will back off to a lower level of detail or do a hard stop
     * on loading to meet this criteria.
     */
    public native void setMaxTiles(int maxTiles);

    /**
     * Maximum number of tiles to display at once.
     * The system will back off to a lower level of detail or do a hard stop
     * on loading to meet this criteria.
     */
    public native int getMaxTiles();

    /**
     * Size of a tile in scren space (pixels^2).
     * Anything taking up less space than this will not be loaded.
     */
    public native void setMinImportance(double minImport);

    /**
     * Size of a tile in scren space (pixels^2).
     * Anything taking up less space than this will not be loaded.
     */
    public native double getMinImportance();

    /**
     * Normally we always load the lowest level, but this can
     * be set to a screen importance as in setMinImportance.
     * In that case we don't load the lowest level unless it
     * exceeds that size.
     */
    public native void setMinImportanceTop(double minImportTop);

    /**
     * Normally we always load the lowest level, but this can
     * be set to a screen importance as in setMinImportance.
     * In that case we don't load the lowest level unless it
     * exceeds that size.
     */
    public native double getMinImportanceTop();

    /**
     * Set the minImportance for a specific level.
     * Useful for tweaking different levels to be loaded at different
     * points.
     */
    public native void setMinImportance(double minImport,int level);

    /**
     * If set, we'll generate geometry to cover the poles on a globe.
     * Turn this off for flat systems (e.g. the map)
     */
    public native void setCoverPoles(boolean coverPoles);

    /**
     * If set, we'll generate geometry to cover the poles on a globe.
     * Turn this off for flat systems (e.g. the map)
     */
    public native boolean getCoverPoles();

    /**
     * If set, we'll generate skirt geometry between the edges of tiles.
     * This works well for the globe.
     */
    public native void setEdgeMatching(boolean edgeMatching);

    /**
     * If set, we'll generate skirt geometry between the edges of tiles.
     * This works well for the globe.
     */
    public native boolean getEdgeMatching();

    /**
     * Each tile will be tesselated into a given number of X and Y grid
     * points.
     */
    public native void setTesselation(int tessX,int tessY);

    /**
     * If set, we'll always make sure the min level is loaded in
     * to provide some background.  Turn it off for true single level
     * loading.  Not great for basemaps.  On by default.
     */
    public native void setForceMinLevel(boolean force);

    /**
     * Return the tesselation in X for a single tile.
     */
    public native int getTesselationX();

    /**
     * Return the tesselation in Y for a single tile
     */
    public native int getTesselationY();

    /**
     * If set, we'll concentrate on loading a single level of detail,
     * with restrictions based on the level loads.
     */
    public native void setSingleLevel(boolean singleLevel);

    /**
     * If set, we'll concentrate on loading a single level of detail,
     * with restrictions based on the level loads.
     */
    public native boolean getSingleLevel();

    /**
     * Detail the levels you want loaded in target level mode.
     * The layer calculates the optimal target level.
     * The entries in this array are relative to that level or absolute.
     * For example [0,-4,-2] means the layer will always try to load levels 0, targetLevel-4
     * and targetLevel-2, but only the latter two if they make sense.
     */
    public native void setLevelLoads(int[] levels);

    /**
     * If set, we won't load any tiles outside these boundaries.
     */
    public void setClipBounds(Mbr bounds)
    {
        setClipBounds(bounds.ll.getX(),bounds.ll.getY(),bounds.ur.getX(),bounds.ur.getY());
    }

    private native void setClipBounds(double llx,double lly,double urx,double ury);

    @Override
    public boolean equals(Object obj) {
        if (obj == null)
            return false;
        if (!(obj instanceof SamplingParams))
            return false;
        return equalsNative(obj);
    }

    private native boolean equalsNative(Object obj);

    public void finalize()
    {
        dispose();
    }
    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();
    private long nativeHandle;
}
