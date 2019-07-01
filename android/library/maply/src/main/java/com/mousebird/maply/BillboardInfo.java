/*
 *  BillboardInfo.java
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
 * Parameters used to control billboard display.
 */
public class BillboardInfo extends BaseInfo {

    /**
     * Creates an empty billboard info object
     */
    public BillboardInfo() {
        initialise();
    }

    /**
     * Billboard is oriented toward the eye or tied to the ground.
     */
    enum Orient {Eye,Ground}

    /**
     * Set the color used by the geometry.
     * @param color Color in Android format, including alpha.
     */
    public void setColor(int color)
    {
        setColor(Color.red(color)/255.f,Color.green(color)/255.f,Color.blue(color)/255.f,Color.alpha(color)/255.f);
    }

    /**
     * Set color by component.
     */
    public native void setColor(float r, float g, float b, float a);

    private Orient orient = Orient.Ground;

    /**
     * Set the orientation toward the user (eye) or tied to the ground (but also taking user position into account)
     */
    public void setOrient(Orient orient)
    {
        setOrientNative(orient.ordinal());
    }

    private native void setOrientNative(int orient);

    /**
     * Return the billboard orientation.
     */
    public Orient getOrient()
    {
        return orient;
    }

    public void finalize()
    {
        dispose();
    }
    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();

    private String shaderName;
}
