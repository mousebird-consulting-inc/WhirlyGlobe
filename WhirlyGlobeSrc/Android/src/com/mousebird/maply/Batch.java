/*
 *  Batch.java
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

public class Batch {

    public Batch(){
        initialize();
    }

    public void finalise(){
        dispose();
    }

    public native void setBatchID (int batchID);

    public native int getBatchID();

    public native void setOffSet(int offSet);

    public native int getOffSet();

    public native void setLen(int len);

    public native int getLen();

    public native void setActive(boolean active);

    public native boolean getActive();

    public native void setStartTime(double startTime);

    public native double getStartTime();

    static {
        nativeInit();
    }

    private static native void nativeInit();
    native void initialize();
    native void dispose();
    private long nativeHandle;
}
