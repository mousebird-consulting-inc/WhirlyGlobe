/*
 *  Light.java
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

import android.graphics.Color;

/** The Light provides a simple interface to basic lighting within the toolkit.
 * <br>
 * You can have up to 8 lights in the current version of the toolkit.  Obviously this is all shader implementation with OpenGL ES 2.0, so you can always just bypass this and do what you like.  However, the default shaders will look for these lights.
 * The lights are very simple, suitable for the globe, and contain a position, a couple of colors, and a view dependent flag.
 */
public class Light {

    private Point3d pos;
    private boolean viewDependent;
    private float[] ambient;
    private float[] diffuse;

    public Light() {
        this.ambient = new float[4];
        this.diffuse =new float[4];
    }

    /** The location of this particular light in display space.
     * <br>
     * This is a single light's location in display space.  Display space for the globe is based on a radius of 1.0.
     */
    public Point3d getPos() {
        return pos;
    }

    /** The location of this particular light in display space.
     * <br>
     * This is a single light's location in display space.  Display space for the globe is based on a radius of 1.0.
     */
    public void setPos(Point3d pos) {
        this.pos = pos;
    }

    /** Controls whether the light takes the model matrix into account or not.
     * <br>
     * If set, this light moves with the model (usually the globe).  You'd use this for a real sun position. If not set, the light is static and does not move or rotate.
     */
    public boolean isViewDependent() {
        return viewDependent;
    }

    /** Controls whether the light takes the model matrix into account or not.
     * <br>
     * If set, this light moves with the model (usually the globe).  You'd use this for a real sun position. If not set, the light is static and does not move or rotate.
     */
    public void setViewDependent(boolean viewDependent) {
        this.viewDependent = viewDependent;
    }

    /** Ambient color for the light.
     * <br>
     * This color will always be added to any given pixel.  It provides a baseline lighting value.
     */
    public void setAmbient(int color) {
        setAmbient(Color.red(color), Color.green(color), Color.blue(color), Color.alpha(color));
    }

    /** Ambient color for the light.
     * <br>
     * This color will always be added to any given pixel.  It provides a baseline lighting value.
     */
    public void setAmbient(float r, float g, float b, float a) {
        if (this.ambient == null)
            this.ambient = new float[4];
        this.ambient[0] = r;
        this.ambient[1] = g;
        this.ambient[2] = b;
        this.ambient[3] = a;
    }

    /** Ambient color for the light.
     * <br>
     * This color will always be added to any given pixel.  It provides a baseline lighting value.
     */
    public float[] getAmbient() {
        return ambient;
    }

    /** Ambient color for the light.
     * <br>
     * This color will always be added to any given pixel.  It provides a baseline lighting value.
     */
    public void setAmbient(float[] ambient) {
        this.ambient = ambient;
    }

    /** Diffuse light color.
     * <br>
     * The diffuse color is multiplied by a directional value and so will vary depending on geometry normals.
     */
    public void setDiffuse(int color) {
        setDiffuse(Color.red(color), Color.green(color), Color.blue(color), Color.alpha(color));
    }

    /** Diffuse light color.
     * <br>
     * The diffuse color is multiplied by a directional value and so will vary depending on geometry normals.
     */
    public void setDiffuse(float r, float g, float b, float a){
        if (this.diffuse == null)
            this.diffuse = new float[4];
        this.diffuse[0] = r;
        this.diffuse[1] = g;
        this.diffuse[2] = b;
        this.diffuse[3] = a;
    }

    /** Diffuse light color.
     * <br>
     * The diffuse color is multiplied by a directional value and so will vary depending on geometry normals.
     */
    public float[] getDiffuse() {
        return diffuse;
    }

    /** Diffuse light color.
     * <br>
     * The diffuse color is multiplied by a directional value and so will vary depending on geometry normals.
     */
    public void setDiffuse(float[] diffuse) {
        this.diffuse = diffuse;
    }

}
