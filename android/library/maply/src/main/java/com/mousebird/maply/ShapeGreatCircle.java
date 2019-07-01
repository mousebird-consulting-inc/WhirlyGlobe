/*
 *  ShapeSphere.java
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

import android.graphics.drawable.ShapeDrawable;

/**
 *     Represents an great circle or great circle with height.
 *     <br>
 *     Great circles are the shortest distance between two points on a globe.
 *     We extend that a bit here, by adding height.
 *     The result is a curved object that can either sit on top of the globe or rise
 *     above it.  In either case it begins and ends at the specified points on the globe.
 **/
public class ShapeGreatCircle extends Shape {

    /**
     * Construct an empty sphere.  Be sure to fill in the fields.
     */
    public ShapeGreatCircle() {
        initialise();
    }

    public void finalize() {
        dispose();
    }

    /**
     * Starting and ending points in geographic radians.
     */
    public native void setPoints(Point2d startLoc,Point2d endLoc);

    /**
     *     Height of the great circle shape right in its middle.
     *     <br>
     *     This is the height of the great circle right in the middle.
     *     It will built toward that height and then go back down as it reaches the endPt.
     *     The height is in display units.  For the globe that's based on a radius of 1.0.
     */
    public native void setHeight(double height);

    /**
     * If set, the sampling value used when breaking the great circle down into
     * segments.
     */
    public native void setSamplingEpsilon(double eps);

    /**
     * If set, we'll do a static number of samples, rather than dynamically calculating them.
     */
    public native void setSamplingStatic(int numSamples);

    /**
     * Angle between start and end points in radians.
     * Useful for calculating a reasonable height.
     */
    public native double angleBetween();

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native public void dispose();
}
