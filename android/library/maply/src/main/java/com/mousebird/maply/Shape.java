/*  Shape.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2021 mousebird consulting
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
 */
package com.mousebird.maply;

import android.graphics.Color;

/** Shape is the base class for the actual shape objects.
 * <br>
 * The maply shape is just the base class.  Look to <ref>ShapeCircle</ref>, <ref>ShapeCylinder</ref>,
 * <ref>ShapeSphere</ref>, <ref>ShapeGreatCircle</ref>, and <ref>ShapeLinear</ref>.
 */
public abstract class Shape {

    protected Shape() {
    }

    /**
     * @return Unique ID for selection
     */
    public native long getSelectID();

    /**
     * @param selectID Unique ID for selection
     */
    public native void setSelectID(long selectID);

    /**
     * The object is selectable if this is set when the object is passed in to an add call.
     * If not set, you'll never see it in selection.
     */
    public native boolean isSelectable();

    /**
     * The object is selectable if this is set when the object is passed in to an add call.
     * If not set, you'll never see it in selection.
     */
    public native void setSelectable(boolean selectable);

    /**
     * Color of the shape
     */
    public void setColor(int color) {
        setColor(Color.red(color),Color.green(color),Color.blue(color),Color.alpha(color));
    }

    /**
     * Color of the shape
     */
    public void setColor(float r, float g, float b, float a) {
        setColorInt((int)(r*255.0f),(int)(g*255.0f),(int)(b*255.0f),(int)(a*255.0f));
    }

    /**
     * Color of the shape
     */
    public native void setColorInt(int r, int g, int b, int a);

    /** If set, this shape is in clip coordinates and will not be transformed.
     * <br>
     * Some objects (the rectangle) can be used as overlays in clip coordinates.  This is set if that's the case.
     */
    public native void setClipCoords(boolean clipVal);

    // Overridden by subclasses
    public abstract void dispose();

    static {
        nativeInit();
    }
    private static native void nativeInit();

    @SuppressWarnings("unused")	// Used from JNI
    protected long nativeHandle;
}
