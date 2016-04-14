/*
 *  Matrix3d.java
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


public class Matrix3d {

    public Matrix3d() {
        initialise();
    }

    public void finalize() {
        dispose();
    }

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();
    /**
     * Return the inverse of this matrix.
     */
    public native Matrix3d inverse();

    /**
     * Transpose and return the matrix.
     */
    public native Matrix3d transpose();
    /**
     * Multiply the vector by this matrix and return the result.
     */
    public native Point3d multiply(Point3d vec);

    public native Matrix3d multiply(Matrix3d matrix);
    private long nativeHandle;
}
