/*
 *  Drawable.java
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

public class Drawable {

    public Drawable(String name){
        initialise(name);
    }

    public void finalise(){
        dispose();
    }

    public native Mbr getLocalMbr();

    public native int getDrawPriority();

    public native long getProgram();

    public native boolean isOn(RendererFrameInfo frameInfo);

    public native void setupGL(WhirlyKitGLSetupInfo setupInfo, OpenGLMemManager memManager);

    public native void tearDownGL(OpenGLMemManager memManager);

    public native void draw(RendererFrameInfo frameInfo, Scene scene);

    public native int getType();

    public native boolean hasAlpha(RendererFrameInfo frameInfo);

    public native Matrix4d getMatrix();

    public native boolean getRequestZBuffer();

    public native boolean getWriteZBuffer();

    public native void updateRenderer(SceneRendererES renderer);

    public native void addTweaker (DrawableTweaker tweaker);

    public native void removeTweaker (DrawableTweaker tweaker);

    public native void runTweakers(RendererFrameInfo frame);

    native void initialise(String name);

    native void dispose();

    static {
        nativeInit();
    }
    private static native void nativeInit();
    private long nativeHandle;


}
