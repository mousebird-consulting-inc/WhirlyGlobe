/*  ShapeInfo.java
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

/**
 * Shape Info is used to hold parameter values for a group of Shape objects.
 * It's read by the system to determine how to turn them into geometry for rendering.
 */
public class ShapeInfo extends BaseInfo {

    public ShapeInfo() {
        initialise();
        setColor(Color.WHITE);
        setZBufferRead(true);
        setZBufferWrite(true);
        setDrawPriority(80000);
    }

    public void finalize() {
        dispose();
    }

    /**
     * Color of the shape
     */
    public void setColor(int color) {
        setColorInt(Color.red(color),Color.green(color),Color.blue(color),Color.alpha(color));
    }

    /**
     * Color of the shape
     */
    public void setColor(float r, float g, float b, float a) {
        setColorInt((int)(r * 255.0f),(int)(g * 255.0f),(int)(b * 255.0f),(int)(a * 255.0f));
    }

    /**
     * Color of the shape
     */
    public native void setColorInt(int r, int g, int b, int a);

    /**
     * If the shape is made of lines, this is the line width.
     */
    public native void setLineWidth(float lineWidth);

    /**
     * Some shapes can be generated inside out (like spheres).
     * This is used in really esoteric cases, like the atmosphere.
     */
    public native void setInsideOut(boolean insideOut);

    /**
     * If set, the center controls the origin for the shapes as they are created.
     * If not set, a center will be calculated for a group of shapes.
     */
    public native void setCenter(Point3d center);

    static {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();
}
