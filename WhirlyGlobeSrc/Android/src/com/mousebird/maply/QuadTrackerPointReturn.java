/*
 *  QuadTrackerPointReturn.java
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

public class QuadTrackerPointReturn {

    public QuadTrackerPointReturn()
    {
        initialise();
    }

    public void finalise()
    {
        dispose();
    }

    public native void setScreenU(double screenU);

    public native double getScreenU();

    public native void setScreenV(double screenV);

    public native double getScreenV();

    public native void setMaplyTileID (int x, int y, int level);

    public native int[] getMaplyTileID();

    public native void setPadding(int padding);

    public native int getPadding();

    public native void setLocX(double locX);

    public native double getLocX();

    public native void setLocY(double locY);

    public native double getLocY();

    public native void setTileU(double tileU);

    public native double getTileU();

    public native void setTileV(double tileV);

    public native double getTileV();


    static {
        nativeInit();
    }

    private static native void nativeInit();
    native void initialise();
    native void dispose();
    private long nativeHandle;
}
