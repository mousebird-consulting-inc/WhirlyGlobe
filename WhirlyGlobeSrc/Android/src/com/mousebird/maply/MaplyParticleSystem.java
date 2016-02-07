/*
 *  MaplyParticleSystem.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro on 26/1/16
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

import android.graphics.Bitmap;

import java.util.ArrayList;
import java.util.Date;
import java.util.Vector;

public class MaplyParticleSystem {

    private String name;
    private ParticleSystem.STATE type;
    private String shader;
    private double lifeTime;
    private double baseTime;
    private int totalParticles;
    private int batchSize;
    private long ident;
    private Vector<Bitmap> images;
    private int defaultDrawPriority = 55000;

    public float getPointSize() {
        return pointSize;
    }

    public void setPointSize(float pointSize) {
        this.pointSize = pointSize;
    }

    private float pointSize = (float)4.0;

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public ParticleSystem.STATE getType() {
        return type;
    }

    public void setType(ParticleSystem.STATE type) {
        this.type = type;
    }

    public String getShader() {
        return shader;
    }

    public void setShader(String shader) {
        this.shader = shader;
    }

    public double getLifeTime() {
        return lifeTime;
    }

    public void setLifeTime(double lifeTime) {
        this.lifeTime = lifeTime;
    }

    public double getBaseTime() {
        return baseTime;
    }

    public void setBaseTime(double baseTime) {
        this.baseTime = baseTime;
    }

    public int getTotalParticles() {
        return totalParticles;
    }

    public void setTotalParticles(int totalParticles) {
        this.totalParticles = totalParticles;
    }

    public void setBatchSize(int batchSize) {
        this.batchSize = batchSize;
    }

    public long getIdent() {
        return ident;
    }

    public void setIdent(long ident) {
        this.ident = ident;
    }

    public Vector<Bitmap> getImages() {
        return images;
    }

    public void setImages(Vector<Bitmap> images) {
        this.images = images;
    }

    public int getDefaultDrawPriority() {
        return defaultDrawPriority;
    }

    public void setDefaultDrawPriority(int defaultDrawPriority) {
        this.defaultDrawPriority = defaultDrawPriority;
    }

    public void setAttrs(ArrayList<ParticleSystemAttribute> attrs) {
        this.attrs = attrs;
    }

    public ArrayList<ParticleSystemAttribute> getAttrs() {
        return attrs;
    }

    private ArrayList<ParticleSystemAttribute> attrs;

    public int getBatchSize() {
        return batchSize;
    }

    public MaplyParticleSystem(String name) {
        this.ident = Identifiable.genID();
        this.name = name;
        this.type = ParticleSystem.STATE.ParticleSystemPoint;
        this.lifeTime = 5.0;
        this.batchSize = 2000;
        this.totalParticles = 100000;
        this.baseTime = new Date().getTime();
        this.attrs = new ArrayList<>();
        this.images = new Vector<>();
    }

    public void addAttribute(String attrName, ParticleSystemAttribute.MaplyShaderAttrType type) {
        ParticleSystemAttribute attr = new ParticleSystemAttribute();
        attr.setName(attrName);
        attr.setType(type);
        attrs.add(attr);
    }

    public void addTexture (Bitmap image) {
        this.images.add(image);
    }

}
