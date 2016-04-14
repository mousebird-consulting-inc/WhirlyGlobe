/*
 *  Affine2d.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2016 mousebird consulting
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


public class Affine2d {

    public static final int TYPE_SCALE = 1;
    public static final int TYPE_TRANS = 2;

    public Affine2d(double x, double y, int type) {
        initialise(x,y, type);
    }

    public void finalise() {
        dispose();
    }


    public native Matrix3d matrix();

    public native Point2d multiply (Point2d pt);

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise(double x, double y, int type);
    native void dispose();

    private long nativeHandle;

}
