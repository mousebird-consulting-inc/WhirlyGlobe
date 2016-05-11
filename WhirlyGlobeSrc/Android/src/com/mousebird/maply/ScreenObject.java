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

public class ScreenObject {

    public class BoundingBox {
        public Point2d ll = new Point2d();
        public Point2d ur = new Point2d();
    }

    public ScreenObject() {
        initialise();
    }

    public void finalise() {
        dispose();
    }

    public native void addPoly(SimplePoly poly);

    public native void addString(StringWrapper string);

    public void addTexture(MaplyTexture tex, float[] color, float width, float height)
    {
        addTextureNative(tex.texID,color[0],color[1],color[2],color[3],width,height);
    }

    public native void addTextureNative(long texID,float red,float green,float blue,float alpha,float width,float height);

    public native void addScreenObject(ScreenObject screenObject);

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

    public void translateX(double x, double y) {

        Matrix3d mat = Matrix3d.translate(x,y);

        transform(mat);
    }

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
