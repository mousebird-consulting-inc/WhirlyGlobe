/*
 *  Shape.java
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

import android.graphics.Color;

/** Shape is the base class for the actual shape objects.
 * <br>
 * The maply shape is just the base class.  Look to ShapeCircle, ShapeCylinder, ShapeSphere, ShapeGreatCircle, and ShapeLinear.
 */
public class Shape {

    protected Shape() {
    }

    /**
      * The object is selectable if this is set when the object is passed in to an add call.  If not set, you'll never see it in selection.
     */
    public native boolean isSelectable();

    /**
     * The object is selectable if this is set when the object is passed in to an add call.  If not set, you'll never see it in selection.
     */
    public native void setSelectable(boolean selectable);

    /**
     * Color of the shape
     */
    public void setColor(int color)
    {
        setColor(Color.red(color)/255.f,Color.green(color)/255.f,Color.blue(color)/255.f,Color.alpha(color)/255.f);
    }

    /**
     * Color of the shape
     */
    public native void setColor(float r, float g, float b, float a);

    /**
     * Color of the shape
     */
    public native float[] getColor();


    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    protected long nativeHandle;
}
