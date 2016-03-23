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

import java.util.List;


public class Light {

    private Point3d pos;
    private boolean viewDependent;
    private float[] ambient;
    private float[] diffuse;

    public Light() {
        this.ambient = new float[4];
        this.diffuse =new float[4];
    }

    public Point3d getPos() {
        return pos;
    }

    public void setPos(Point3d pos) {
        this.pos = pos;
    }

    public boolean isViewDependent() {
        return viewDependent;
    }

    public void setViewDependent(boolean viewDependent) {
        this.viewDependent = viewDependent;
    }

    public void setAmbient(int color) {
        setAmbient(Color.red(color), Color.green(color), Color.blue(color), Color.alpha(color));
    }

    public void setAmbient(float r, float g, float b, float a) {
        if (this.ambient == null)
            this.ambient = new float[4];
        this.ambient[0] = r;
        this.ambient[1] = g;
        this.ambient[2] = b;
        this.ambient[3] = a;
    }

    public float[] getAmbient() {
        return ambient;
    }

    public void setAmbient(float[] ambient) {
        this.ambient = ambient;
    }
    
    public void setDiffuse(int color) {
        setDiffuse(Color.red(color), Color.green(color), Color.blue(color), Color.alpha(color));
    }

    public void setDiffuse(float r, float g, float b, float a){
        if (this.diffuse == null)
            this.diffuse = new float[4];
        this.diffuse[0] = r;
        this.diffuse[1] = g;
        this.diffuse[2] = b;
        this.diffuse[3] = a;
    }

    public float[] getDiffuse() {
        return diffuse;
    }

    public void setDiffuse(float[] diffuse) {
        this.diffuse = diffuse;
    }

}
