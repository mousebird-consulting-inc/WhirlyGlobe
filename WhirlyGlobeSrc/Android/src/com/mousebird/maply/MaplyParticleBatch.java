/*
 *  MaplyParticleBatch.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro on 21/1/16
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

import java.nio.ByteBuffer;
import java.util.Date;
import java.util.Vector;

public class MaplyParticleBatch {

    private MaplyParticleSystem partSys;
    private double time;
    private Vector<ParticleSystemAttrVals> attrVals;

    public MaplyParticleBatch (MaplyParticleSystem partSys) {
        this.partSys = partSys;
        this.time = new Date().getTime();
        this.attrVals =  new Vector<>();
    }

    private class ParticleSystemAttrVals {
        public double attrID;
        public ByteBuffer data;
    }

    public boolean addAttribute(String attrName, ByteBuffer data) {
        for (ParticleSystemAttribute attr : this.partSys.getAttrs()) {
            if (attrName.equals(attr.getName())) {
                // Found. Now make sure the size matches
                ParticleSystemAttrVals attrValue = new ParticleSystemAttrVals();
                attrValue.attrID = attr.ident;
                attrValue.data = data;
                if (data.capacity() != attr.getTypeSize() * partSys.getBatchSize()) {
                    return false;
                }
                this.attrVals.add(attrValue);
                return true;
            }
        }
        return false;
    }

    private boolean isValid() {
        return partSys.getAttrs().size() == this.attrVals.size();
    }
}
