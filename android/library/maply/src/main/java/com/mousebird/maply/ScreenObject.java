/*
 *  ScreenObject.java
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

import android.graphics.Bitmap;

/**
 * The Maply Screen Object is used to build up a more complex screen
 * object from multiple pieces.
 * <br>
 * You can use one or more of these to build up a combination of labels
 * and images that form a single marker, label, or billboard.
 */
public class ScreenObject {

    public class BoundingBox {
        public Point2d ll = new Point2d();
        public Point2d ur = new Point2d();
    }

    public ScreenObject() {
        initialise();
    }

    public void finalize() {
        dispose();
    }

    public native void addPoly(SimplePoly poly);

	/**
     * Add a string to the screen object
     * @param string the string to add
     */
    public native void addString(StringWrapper string);

    /**
     * Add a rectangle of the given size and stretch the given texture over it.
     * @param tex Texture to use on the rectangle.
     * @param color Color to make the resulting rectangle.
     * @param width Width of the rectangle to create.
     * @param height Height of the rectangle to create.
     */
    public void addTexture(MaplyTexture tex, float[] color, float width, float height)
    {
        addTextureNative(tex.texID,color[0],color[1],color[2],color[3],width,height);
    }

    public native void addTextureNative(long texID,float red,float green,float blue,float alpha,float width,float height);

	/**
     * Add the contents of the given screen object to this screen object.
     * @param screenObject
     */
    public native void addScreenObject(ScreenObject screenObject);

	/**
     * Calculate and return the current bounding box of the screen object.
     * @return the current bounding box of the screen object.
     */
    public BoundingBox getSize() {
        BoundingBox bbox = new BoundingBox();
        getSizeNative(bbox.ll,bbox.ur);

        return bbox;
    }

    public native void getSizeNative(Point2d ll,Point2d ur);

    public void scale(double x, double y) {

        Matrix3d mat = Matrix3d.scale(x,y);

        transform(mat);
    }

	/**
     * Apply a translation to all the pieces of the screen object.
     * @param x
     * @param y
     */
    public void translateX(double x, double y) {

        Matrix3d mat = Matrix3d.translate(x,y);

        transform(mat);
    }

    /**
     * Apply the given 2D transform to the screen object.
     * @param mat Matrix to use for transform.
     */
    public native void transform(Matrix3d mat);

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();

    private long nativeHandle;

}
