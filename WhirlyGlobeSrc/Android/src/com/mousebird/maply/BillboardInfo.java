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


/**
 * Parameters used to control billboard display.
 */
public class BillboardInfo extends BaseInfo {

    /**
     * Creates an empty billboard info
     */
    public BillboardInfo() {
        initialise();
    }

    /**
     * TODO(sjg)
     * Color components range from 0.0 to 1.0.
     * @param r red component
     * @param g green component
     * @param b blue component
     * @param a alpha component
     */
    public native void setColor(float r, float g, float b, float a);

    /**
     * TODO(sjg)
     * @return the color components (rgba)
     */
    public native float[] getColor();

    /**
     * TODO(sjg)
     * @param zBufferRead
     */
    public native void setZBufferRead(boolean zBufferRead);

    /**
     * TODO(sjg)
     * @return
     */
    public native boolean getZBufferRead();

    /**
     * TODO(sjg)
     * @param zBufferWrite
     */
    public native void setZBufferWrite(boolean zBufferWrite);

    /**
     * TODO(sjg)
     * @return
     */
    public native boolean getZBufferWrite();

    /**
     * @return the shader name to be used in the billboard.
     */
    public String getShaderName() {
        return shaderName;
    }

    /**
     * @param shaderName the shader name to be used in the billboard.
     */
    public void setShaderName(String shaderName) {
        this.shaderName = shaderName;
    }

    public void finalize() {
        dispose();
    }

    static {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();
    private long nativeHandle;
    private String shaderName;
}
