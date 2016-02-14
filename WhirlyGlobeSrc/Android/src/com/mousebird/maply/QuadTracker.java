/*
 *  QuadTracker.java
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

public class QuadTracker {


    private QuadTracker(){
    }
    public QuadTracker(GlobeController globeController){

        initialise(globeController.globeView);
        this.setRenderer(globeController.renderWrapper.maplyRender);
        this.setAdapter(globeController.coordAdapter);
    }

    public void finalise(){
        dispose();
    }

    public native QuadTrackerPointReturn[] tiles(QuadTrackerPointReturn[] tilesInfo, int numPts);

    public native void addTile(int x, int y, int level);

    public native void removeTile(int x, int y, int level);

    public native void setCoordSystem (CoordSystem coordSystem, Point2d ll, Point2d ur);

    public native void setMinLevel (int minLevel);

    public native int getMinLevel();

    public native void setAdapter (CoordSystemDisplayAdapter adapter);

    public native void setRenderer (MaplyRenderer renderer);




    static {
        nativeInit();
    }

    private static native void nativeInit();
    native void initialise(GlobeView globeView);
    native void dispose();
    private long nativeHandle;


}
