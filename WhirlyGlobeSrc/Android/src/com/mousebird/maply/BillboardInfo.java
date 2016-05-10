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


public class BillboardInfo extends BaseInfo {

    public BillboardInfo() {
        initialise();
    }

    public native void setColor (float r, float g, float b, float a);

    public native float[] getColor();

    public native void setZBufferRead(boolean zBufferRead);

    public native boolean getZBufferRead();

    public native void setZBufferWrite(boolean zBufferWrite);

    public native boolean getZBufferWrite();

    private String shaderName;

    public String getShaderName() {
        return shaderName;
    }

    public void setShaderName(String shaderName) {
        this.shaderName = shaderName;
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
    private long nativeHandle;
}
