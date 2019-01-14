/*
 *  ShapeManager.java
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

import java.util.List;

/**
 * The Shape Manager is used to create and destroy geometry for shapes
 * like circles, cylinders and so forth.
 * It's entirely thread safe (except for destruction).
 */
public class ShapeManager {

    private ShapeManager() {

    }
    public ShapeManager(Scene scene) {
        initialise(scene);
    }

    public void finalize() {
        dispose();
    }

	/**
     * Add an array of shapes.
     * The returned ID can be used to remove or modify the group of shapes.
     */
    public native long addShapes(List<Shape> shapes,ShapeInfo info, ChangeSet changes);

	/**
     * Remove a group of shapes named by the given ID
     */
    public native void removeShapes(long[] shapesIDs, ChangeSet changes);

	/**
     * Enable/disable a group of shapes
     */
    public native void enableShapes (long[] shapesIDs, boolean enable, ChangeSet changes);

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise(Scene scene);
    native void dispose();
    private long nativeHandle;
}
