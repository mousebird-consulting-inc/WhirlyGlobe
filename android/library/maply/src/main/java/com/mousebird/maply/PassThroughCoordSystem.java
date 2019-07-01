/*
 *  PassThroughCoordSystem.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/19/19.
 *  Copyright 2011-2010 mousebird consulting
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
 * This coordinate system literally passes through the values its
 * given for conversion.  It's used in cases where we don't really have a projection.
 */
public class PassThroughCoordSystem extends CoordSystem
{
    public PassThroughCoordSystem()
    {
        initialise();
    }

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
}
