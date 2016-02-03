/*
 *  OpenGLStateOptimizer.java
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

public class OpenGLStateOptimizer {

    public OpenGLStateOptimizer(){
        initialise();
    }

    public void finalise(){
        dispose();
    }

    static {
        nativeInit();
    }

    private static native void nativeInit();

    native void initialise();
    native void dispose();

    private long nativeHandle;

    public native void setActiveTexture(int activeTexture);

    public native int getActiveTexture();

    public native void setDepthMask (boolean depthMask);

    public native boolean getDepthMask();

    public native void setEnableDepthTest (boolean enable);

    public native boolean getEnableDepthTest();

    public native void setDepthFunc (int depthFuncVal);

    public native int getDepthFuncVal();

    public native void setUseProgram (int progID);

    public native int getUseProgram();

    public native void setLineWidth (float lineWidth);

    public native float getLineWidth();

    public native void reset();
}
