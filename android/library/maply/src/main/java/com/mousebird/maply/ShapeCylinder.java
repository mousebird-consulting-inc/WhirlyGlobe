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

/** Display a cylinder at the given location.
 */
public class ShapeCylinder extends Shape {

    /**
     * Construct an empty sphere.  Be sure to fill in the fields.
     */
    public ShapeCylinder() {
        initialise();
    }

    public void finalize() {
        dispose();
    }

    /** Center of the sphere in local coordinates.
     * <br>
     *  The x and y coordinates correspond to longitude and latitude and are in geographic (radians).  The Z value is in display coordinates.  For that globe that's based on a radius of 1.0.  Scale accordingly.
     */
    public native void setBaseCenter (Point2d loc);

    /**
     * Height offset of the base of the cylinder.
     */
    public native void setBaseHeight(double height);

    /**
     * Height of the cylinder from its base.
     */
    public native void setHeight(double height);

    /** Radius of the cylinder in display units.
     * <br>
     * This is the radius of the cylinder, but not in geographic.  It's in display units.  Display units for the globe are based on a radius of 1.0.  Scale accordingly.  For the map, display units typically run from -PI to +PI, depending on the coordinate system.
     */
    public native void setRadius(double radius);

    /**
     * Number of samples used in generating the cylinder.
     */
    public native void setSample(int sample);

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native public void dispose();
}
