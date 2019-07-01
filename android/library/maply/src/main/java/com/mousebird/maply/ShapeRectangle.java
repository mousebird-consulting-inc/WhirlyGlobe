/*
 *  ShapeSphere.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2016 mousebird consulting
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

/** Display a rectangle at the given location.
 */
public class ShapeRectangle extends Shape {

    /**
     * Construct an empty sphere.  Be sure to fill in the fields.
     */
    public ShapeRectangle() {
        initialise();
    }

    public void finalize() {
        dispose();
    }

    /**
     * Set the lower left and upper right points for the rectangle.
     */
    public native void setPoints(Point3d ll,Point3d ur);

    /**
     * Assign a texture for display.
     */
    public void addTexture(MaplyTexture tex)
    {
        addTextureID(tex.texID);
    }

    /**
     * Assign the texture by unique ID.
     */
    native void addTextureID(long texID);

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native public void dispose();
}
