/*
 *  SceneRendererES.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2014 mousebird consulting
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



import java.util.Vector;

public class SceneRendererES {


    public static final int ZBufferMode_ON = 1;
    public static final int ZBufferMode_OFF = 2;
    public static final int ZBufferMode_DEFAULT = 3;

    public SceneRendererES(int apiVersion){
        initialise(apiVersion);
    }

    public void finalise(){
        dispose();
    }

    static {
        nativeInit();
    }

    private static native void nativeInit();

    native void initialise(int apiVersion);
    native void dispose();

    private long nativeHandle;

    public native void setup();

    public native void setRenderUntil(double newTime);

    public native void addContinuousRenderRequest(double drawID);

    public native void removeContinuosRenderRequest(double drawID);

    public native void forceDrawNextFrame();

    public native void setClearColor(float r, float g, float b, float a);

    public native Point2f getFrameBufferSize();

    public native Scene getScene();

    public native View getView();

    public native float getScale();

    public native void findDrawables (Cullable cullable, GlobeView globeView, Point2f frameSize, Matrix4d modelTrans, Point3d eyeVec, RendererFrameInfo frameInfo, Mbr screenMbr, boolean isTopLevel, Vector<Drawable> toDraw, int drawablesConsidered);

    public native boolean viewDidChange();

    public native void setTriggerDraw();

    public native void setZBufferMode(int inZBufferMode);

    public native void setScene(Scene newScene);

    public native void setDoCulling(boolean newCull);

    public native void setPerfInterval (int howLong);

    public native void setUseViewChanged (boolean newVal);

    public native void setView (View newView);

    public native void setExtraFrameMode(boolean newMode);

}
