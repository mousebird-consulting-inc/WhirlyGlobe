/*
 *  ShapeInfo.java
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
    }

    public void finalise() {
        dispose();
    }

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

    /**
     * If the shape is made of lines, this is the line width.
     */
    public native void setLineWidth(float lineWidth);

    /**
     * If the shape is made of lines, this is the line width.
     */
    public native float getLineWidth();

    /**
     * Some shapes can be generated inside out (like spheres).
     * This is used in really esoteric cases, like the atmosphere.
     */
    public native void setInsideOut(boolean insideOut);

    /**
     * Some shapes can be generated inside out (like spheres).
     * This is used in really esoteric cases, like the atmosphere.
     */
    public native boolean getInsideOut();

    /**
     * If set, we'll read from the z buffer before rendering.
     * If not set, only drawPriority matters.
     */
    public native void setZBufferRead(boolean zBufferRead);

    /**
     * If set, we'll read from the z buffer before rendering.
     * If not set, only drawPriority matters.
     */
    public native boolean getZBufferRead();

    /**
     * If set, we'll write to the z buffer as the shape is rendered.
     * If not set, the shape won't modify the z buffer.
     */
    public native void setZBufferWrite(boolean zBufferWrite);

    /**
     * If set, we'll write to the z buffer as the shape is rendered.
     * If not set, the shape won't modify the z buffer.
     */
    public native boolean getZBufferWrite();

    /**
     * If set, the center controls the origin for the shapes as they are created.
     * If not set, a center will be calculated for a group of shapes.
     */
    public native Point3d getCenter();

    /**
     * If set, the center controls the origin for the shapes as they are created.
     * If not set, a center will be calculated for a group of shapes.
     */
    public native void setCenter(Point3d center);

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();
}
