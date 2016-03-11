/*
 *  ShapeSphere.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2015 mousebird consulting
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

public class ShapeSphere extends Shape {

    public ShapeSphere() {
        initialise();
    }

    public void finalise() {
        dispose();
    }

    public native void setLoc (Point2d loc);

    public native Point2d getLoc();

    public native float getHeight();

    public native void setHeight(float height);

    public native float getRadius();

    public native void setRadius(float radius);

    public native void setSampleX(int sampleX);

    public native int getSampleX();

    public native void setSampleY(int sampleY);

    public native int getSampleY();

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();
    private long nativeHandle;
}
