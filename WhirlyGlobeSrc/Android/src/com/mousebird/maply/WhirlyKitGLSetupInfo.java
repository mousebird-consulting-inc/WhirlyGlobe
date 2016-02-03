/*
 *  WhirlyKitGLSetupInfo.java
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

public class WhirlyKitGLSetupInfo {


    public WhirlyKitGLSetupInfo(){
        initialise();
    }

    public void finalise(){
        dispose();
    }

    public native void setMinZres (float minZres);

    public native float getMinZres ();

    native void initialise();

    native void dispose();

    static
    {
        nativeInit();
    }

    private static native void nativeInit();
    private long nativeHandle;
}
