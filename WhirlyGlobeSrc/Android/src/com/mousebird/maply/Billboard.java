/*
 *  Billboard.java
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
import android.graphics.Color;
import android.util.Size;

import java.util.ArrayList;
import java.util.List;


/**
 * Single billboard representation.  Billboards are oriented towards
 * the user.  Fill this out and hand it over to the billboard layer
 * to manage.
 */
public class Billboard {

    /// Billboard orientation
    public static final String MAPLY_BILLBOARD_ORIENTE = "billboardorient";
    /// Billboards are oriented toward the eye, but rotate on the ground
    public static final String MAPLY_BILLBOARD_ORIENTE_GROUND = "billboardorientground";
    /// Billboards are oriented only towards the eye
    public static final String MAPLY_BILLBOARD_ORIENTE_EYE = "billboardorienteye";


    public Billboard() {
        initialise();
    }

    /**
     * @param center center in display coordinates
     */
    public native void setCenter(Point3d center);

    /**
     * @return center in display coordinates
     */
    public native Point3d getCenter();

    /**
     * @param size size (for selection)
     */
    public native void setSize(Point2d size);

    /**
     * @return Size (for selection)
     */
    public native Point2d getSize();

    /**
     * If set, this marker should be made selectable and it will be
     * if the selection layer has been set
     * @param selectable
     */
    public native void setSelectable(boolean selectable);

    /**
     * @return the selectable flag
     */
    public native boolean getSelectable();

    public ScreenObject getScreenObject() {
        return screenObject;
    }

    public void setScreenObject(ScreenObject screenObject) {
        this.screenObject = screenObject;
    }

    public ArrayList<VertexAttribute> getVertexAttributes() {
        return vertexAttributes;
    }

    public void setVertexAttributes(ArrayList<VertexAttribute> vertexAttributes) {
        this.vertexAttributes = vertexAttributes;
    }

    /**
     * @return Unique ID for selection
     */
    public long getSelectID() {
        return selectID;
    }

    /**
     * @param selectID Unique ID for selection
     */
    public void setSelectID(long selectID){
        this.selectID = selectID;
    }

    public native void addPoly(List<Point2d> points, List<Point2d> texCoords, float[] color, List<VertexAttribute> vertexAttributes, long texID);

    public void finalize()
    {
        dispose();
    }

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();
    private long nativeHandle;

    private long selectID = Identifiable.genID();
    private ScreenObject screenObject;
    private ArrayList<VertexAttribute> vertexAttributes = new ArrayList<>();

}
