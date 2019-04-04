/*
 *  LoftedPolyManager.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/4/19.
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

/**
 * The Lofted Polygon Manager is an interface to the Maply C++ manager
 * and should be invisible to toolkit users.
 */
class LoftedPolyManager {
    private LoftedPolyManager()
    {
    }

    LoftedPolyManager(Scene scene) { initialise(scene); }

    // Add vectors to turn into lofted polygons
    public native long addPolys(VectorObject[] vecs,LoftedPolyInfo polyInfo,ChangeSet changes);

    public void finalize()
    {
        dispose();
    }
    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise(Scene scene);
    native void dispose();
    private long nativeHandle;
}
