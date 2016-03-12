/*
 *  ShapeInfo.java
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


public class ShapeInfo extends BaseInfo {

    public ShapeInfo() {
        initialise();
    }

    public void finalise() {
        dispose();
    }

    public native void setColor(float r, float g, float b, float a);

    public native float[] getColor();

    public native void setLineWidth(float lineWidth);

    public native float getLineWidth();

    public native void setId(long id);

    public native long getId();

    public native void setInsideOut(boolean insideOut);

    public native boolean getInsideOut();

    public native void setZBufferRead(boolean zBufferRead);

    public native boolean getZBufferRead();

    public native void setZBufferWrite(boolean zBufferWrite);

    public native boolean getZBufferWrite();

    public native boolean hasCenter();

    public native void setHasCenter(boolean hasCenter);

    public native Point3d getCenter();

    public native void setCenter(Point3d center);

    private long sampleX;

    private long sampleY;

    private long drawPriority;

    private String shader;

    public long getSampleX() {
        return sampleX;
    }

    public void setSampleX(long sampleX) {
        this.sampleX = sampleX;
    }

    public long getSampleY() {
        return sampleY;
    }

    public void setSampleY(long sampleY) {
        this.sampleY = sampleY;
    }

    public long getDrawPriority() {
        return drawPriority;
    }

    public void setDrawPriority(long drawPriority) {
        this.drawPriority = drawPriority;
    }

    public String getShader() {
        return shader;
    }

    public void setShader(String shader) {
        this.shader = shader;
    }

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();
    private long nativeHandle;
}
