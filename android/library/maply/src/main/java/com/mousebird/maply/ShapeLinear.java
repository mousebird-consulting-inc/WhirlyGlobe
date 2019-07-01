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
 *     A linear feature offset from the globe.
 *     <br>
 *     The main difference between this object and a regular MaplyVectorObject is that you
 *     specify coordiantes in 3D.  You can use this to create linear features that are offset
 *     from the globe.
 **/
public class ShapeLinear extends Shape {

    /**
     * Construct an empty sphere.  Be sure to fill in the fields.
     */
    public ShapeLinear() {
        initialise();
    }

    public void finalize() {
        dispose();
    }

    /**
     * The coordinates used to draw the lines.
     * <br>
     * The x and y values are in geographic.  The z values are offsets from the globe (or map)
     * and are in display units.  For the globe display units are based on a radius of 1.0.
     */
    public native void setCoords(Point3d[] coords);

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native public void dispose();
}
