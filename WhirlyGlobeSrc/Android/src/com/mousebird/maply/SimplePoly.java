/*
 *  SimplePoly.java
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

import java.util.ArrayList;
import java.util.List;


/**
 * Extremely simple polygon class
 */
public class SimplePoly {

    public SimplePoly() {
        initialise();
    }

    public SimplePoly(Texture inTexture, float[] color, List<Point2d> pts, List<Point2d> texCoords) {
        initialise(inTexture.getID(), color[0], color[1], color[2], color[3], pts, texCoords);
    }

    native void initialise(long texID, float red, float green, float blue, float alpha, List<Point2d> pts, List<Point2d> texCoords);

    public void finalize() {
        dispose();
    }

    public void addTexture(Texture texture)
    {
        addTextureNative(texture.getID());
    }

    public native void addTextureNative(long texID);

    public native void addColor(float[] color);

    public native float[] getColor();

    public native void addPt(Point2d pt);

    public native void addPts(List<Point2d> pts);

    public native void setPt(int index, Point2d newPt);

    public native void addTexCoord(Point2d texCoord);

    public native void addTexCoords(List<Point2d> texCoord);

    public native void setTexCoord(int index, Point2d newTexCoord);

    public native int getPtsSize();

    public native int getTexCoordsSize();

    public native Point2d getPt (int index);

    public native Point2d getTexCoord(int index);

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();

    private long nativeHandle;
}
