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

/** An extruded shape takes a linear outline which it sweeps from its
 *  base up to a defined height.  Useful for things like 3D arrows.
 */
public class ShapeExtruded extends Shape {

    /**
     * Construct an empty sphere.  Be sure to fill in the fields.
     */
    public ShapeExtruded() {
        initialise();
    }

    public void finalize() {
        dispose();
    }

    /** Center of the extruded shape in geographic.
     * These are geographic radians.
     */
    public native void setLoc (Point2d loc);

    /**
     * The height of the shape's center above the globe.
     * This height is in display coordinates.  1.0 is the radius of the Earth.
     */
    public native void setHeight(double height);

    /**
     * The outline in 2D that makes up the extrusion.
     * By default this is in meters.  See scale.
     */
    public native void setOutline(Point2d[] pts);

    /**
     * Amount to scale the outline coordinates by before creating geometry.
     * By default this is 1/EarthRadius so the outline can be in meters.
     */
    public native void setScale(double scale);

    /**
     * Amount to scale the outline coordinates by before creating geometry.
     * By default this is 1/EarthRadius so the outline can be in meters.
     */
    public native double getScale();

    /**
     * Thickness of the extruded shape.  This is in the same units as the outline.
     */
    public native void setThickness(double thick);

    /**
     * If set, a matrix that can be used to place the feature.
     * We still use the location and scale, so plan accordingly.
     */
    public native void setTransform(Matrix4d mat);

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native public void dispose();
}
