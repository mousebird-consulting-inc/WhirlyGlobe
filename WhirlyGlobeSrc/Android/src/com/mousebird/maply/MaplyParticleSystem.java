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
    private double ident;
    private Vector<Object> images;

    public ArrayList<SingleVertexAttributeInfo> getAttrs() {
        return attrs;
    }

    private ArrayList<SingleVertexAttributeInfo> attrs;

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

    public void addAttribute(String attrName, SingleVertexAttributeInfo.MaplyShaderAttrType type) {
        SingleVertexAttributeInfo attr = new SingleVertexAttributeInfo();
        attr.setName(attrName);
        attr.setType(type.getValue());
        attrs.add(attr);
    }

    public void addTexture (Object image) {
        this.images.add(image);
    }

}
