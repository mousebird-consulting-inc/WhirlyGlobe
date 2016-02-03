/*
 *  Point3f.java
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

public class Point3f {

    /**
     * Initialize empty.
     */
    public Point3f()
    {
        initialise();
    }

    /**
     * Make a copy from the given Point3d
     */
    public Point3f(Point3f that)
    {
        initialise();
        setValue(that.getX(),that.getY(),that.getZ());
    }

    /**
     * Initialize with 3 doubles.
     */
    public Point3f(float x,float y,float z)
    {
        initialise();
        setValue(x,y,z);
    }

    public void finalize()
    {
        dispose();
    }


    /**
     * Return the X value.
     */
    public native float getX();
    /**
     * Return the Y value.
     */
    public native float getY();
    /**
     * Return the Z value.
     */
    public native float getZ();
    /**
     * Set the value of the point.
     */
    public native void setValue(float x,float y,float z);

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();
    private long nativeHandle;

}
