package com.mousebird.maply;

import android.graphics.Bitmap;

/**
 * The Marker is a 2D rectangle plastered on top of the globe or map.  It has a set size in 3D
 * and will not move around. Specific information set within
 * this object and more general information in the associated MarkerInfo.
 */

public class Marker
{
    /**
     * Unique ID used by Maply for selection.
     */
    long ident = Identifiable.genID();

    /**
     * The location in geographic (WGS84) radians.  x is longitude, y is latitude.
     * The marker will track this location.
     */
    public Point2d loc = null;

    /**
     * Size of the marker on screen.
     */
    public Point2d size = new Point2d(16,16);

    /**
     * If set, this is the image we'll use for this marker.
     */
    public Bitmap image = null;

    /**
     * If set we'll animate these images one after the other over the duration.
     */
    public MaplyTexture images[] = null;

    /**
     * If images are passed in, this is the time it will take to cycle through them all.
     * By default this is 5s.
     */
    public double period = 5.0;

    /**
     * Background color for a marker can be overridden at this level.
     */
    public int color = 0xFFFFFFFF;

    /**
     * Turn this on if you want the marker object to be selectable.
     */
    public boolean selectable = false;

    /**
     * For selection, we include a user accessible object pointer.  You use
     * this to identify a marker if you're doing user selection.
     */
    public Object userObject = null;
}
