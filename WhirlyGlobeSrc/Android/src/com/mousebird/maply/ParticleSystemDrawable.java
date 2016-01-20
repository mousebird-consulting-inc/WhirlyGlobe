/*
 *  ParticleSystemDrawable.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro on 19/1/16
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

import java.util.List;
import java.util.Vector;

public class ParticleSystemDrawable {

    private ParticleSystemDrawable() {
    }

    ParticleSystemDrawable(String name, Vector<AttrDictionary> vectAttrs, int numTotalPoints, int batchSize, boolean useRectangles, boolean useInstancing) {
        initialize(name, vectAttrs, numTotalPoints, batchSize, useRectangles, useInstancing);
    }

    public void finalize() {
        dispose();
    }


    /// No bounding box, since these change constantly
    public native Mbr getLocalMbr();

    /// Draw priority for ordering
    public native int getDrawPriority();

    public native void setDrawPriority(int drawPriority);

    /// Program to use for rendering
    public native int getProgram();

    public native void setProgram(int program);

    /// Whether it's currently displaying
    public native boolean isOn (WhirlyKitRendererFrameInfo frameInfo);

    /// True to turn it on, false to turn it off
    public native void setOnOff (boolean onOff);

    /// Set the base time
    public native void setBaseTime (double inBaseTime);

    /// Set the point size
    public native void setPointSize (float inPointSize);

    /// Set the lifetime
    public native void setLifeTime (double inLifeTime);

    public native double getLifeTime();

    /// Set all the textures at once
    public native void setTexIDs (List<Integer> inTexIDs);

    /// Create our buffers in GL
    public native void setupGL (WhirlyKitGLSetupInfo setupInfo, OpenGLMemManager memManager);

    /// Destroy GL buffers
    public native void teardownGL (OpenGLMemManager memManager);

    /// Called on the rendering thread to draw
    public native void draw (WhirlyKitRendererFrameInfo frameInfo, Scene scene);


    /// Don't need to update the renderer particularly
   // void updateRenderer(WhirlyKitSceneRendererES *renderer);

    /// If set, we want to use the z buffer
    public native boolean getRequestZBuffer();

    public native void setRequestZBuffer (boolean enable);

    /// If set, we want to write to the z buffer
    public native boolean getWriteZbuffer();

    public native void setWriteZbuffer(boolean enable);

    public native void addAttributeData (List<AttrDictionary> attrData, Batch batch);

    public native boolean findEmptyBatch (Batch retBatch);

    public native void updateBatches (double now);

    static {
        nativeInit();
    }

    private static native void nativeInit();
    native void initialize(String name, Vector<AttrDictionary> vectAttrs, int numTotalPoints, int batchSize, boolean useRectangles, boolean useInstancing);
    native void dispose();
    private long nativeHandle;

}
