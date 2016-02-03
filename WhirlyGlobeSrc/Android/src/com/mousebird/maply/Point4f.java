/*
 *  Point4f.java
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

public class Point4f {

    /**
     * Initialize empty.
     */
    public Point4f()
    {
        initialise();
    }

    /**
     * Make a copy from the given Point4f
     */
    public Point4f(Point4f that)
    {
        initialise();
        setValue(that.getX(),that.getY(),that.getZ(),that.getW());
    }

    public Point4f(Point3f that,float w)
    {
        initialise();
        setValue(that.getX(),that.getY(),that.getZ(),w);
    }

    /**
     * Initialize with 4 float.
     */
    public Point4f(float x,float y,float z,float w)
    {
        initialise();
        setValue(x,y,z,w);
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
     * Return the W value.
     */
    public native float getW();
    /**
     * Set the value of the point.
     */
    public native void setValue(float x,float y,float z,float w);

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();
    private long nativeHandle;

}
