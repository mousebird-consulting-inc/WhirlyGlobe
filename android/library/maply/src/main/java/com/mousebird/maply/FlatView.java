/*
 *  FlatView.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/19/19.
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
 * The flat view implements a top down orthogonal projection
 *     which is prefect for doing a straight up 2D map.
 *     It thinks more like a window in that it's trying to
 *     display the full extents (as passed in) within a large window
 *     (also passed in) but only showing a smaller window within that.
 *     We presume the caller will move that smaller window around, thus
 *     changing our model and projection matrices.
 */
public class FlatView extends MapView
{
    private FlatView()
    {
    }

    FlatView(MapController inControl,CoordSystemDisplayAdapter inCoordAdapter)
    {
        super(inControl,inCoordAdapter);
    }

    protected FlatView clone()
    {
        FlatView that = new FlatView(control.get(),coordAdapter);
        nativeClone(that);
        return that;
    }

    /**
     * This view tries to display the given extents in display space
     */
    public void setExtents(Mbr mbr)
    {
        setExtentsNative(mbr.ll,mbr.ur);
    }

    protected native void setExtentsNative(Point2d ll,Point2d ur);

    /**
     * Sets the total window size and the region we're looking at within it.
     * This just gets converted to model and projection matrix parameters
     */
    public native void setWindow(Point2d size,Point2d offset);

    public void finalize()
    {
        dispose();
    }
    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise(CoordSystemDisplayAdapter coordAdapter);
    // Make a copy of this map view and return it
    protected native void nativeClone(FlatView dest);
    native void dispose();
}
