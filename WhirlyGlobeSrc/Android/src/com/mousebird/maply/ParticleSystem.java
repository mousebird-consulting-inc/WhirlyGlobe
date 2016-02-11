/*
 *  ParticleSystem.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro on 18/1/16
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

import java.util.Date;

public class ParticleSystem {


    public long id = Identifiable.genID();

    public enum STATE {
        ParticleSystemPoint(0),
        ParticleSystemRectangle(1);

        private final int value;

        STATE(int value) {
            this.value = value;
        }

        public int getValue() {
            return this.value;
        }
    }

    public ParticleSystem(String name) {
        initialise();
        this.setIdent(id);
        this.setName(name);
        this.setParticleSystemType(STATE.ParticleSystemPoint.getValue());
        this.setLifetime(5.0);
        this.setBatchSize(2000);
        this.setTotalParticles(100000);
        this.setBasetime(new Date().getTime());
    }

    public void finalize() {
        dispose();
    }

    private native void setIdent(long ident);

    public long getIdent() {
        return id;
    }

    public native void setName(String name);

    public native void setDrawPriority(int drawPriority);

    public native void setPointSize(float pointSize);

    public native void setParticleSystemType(int particleSystemType);

    public native void setShaderID(long shaderID);

    public native void setLifetime(double lifetime);

    public native void setBasetime(double basetime);

    public native void setBatchSize(int batchSize);

    public native int getBatchSize();


    public native void setTotalParticles(int totalParticles);

    public native void addParticleSystemAttribute(String name, int type);

    public native void addTexID(long texID);

    public ParticleSystemAttribute [] getAttrs() {
        String names[] = this.getAttributesNames();
        int types[] = this.getAttributesTypes();

        if (names.length != types.length) {
            return null;
        }

        ParticleSystemAttribute attrsList[] = new ParticleSystemAttribute[names.length];
        for (int i = 0 ; i < attrsList.length; i++) {
            attrsList[i].setName(names[i]);
            attrsList[i].setType(ParticleSystemAttribute.MaplyShaderAttrType.values()[types[i]]);
        }

        return attrsList;
     }

    private native String[] getAttributesNames();
    private native int[] getAttributesTypes();

    static {
        nativeInit();
    }

    private static native void nativeInit();
    native void initialise();
    native void dispose();
    private long nativeHandle;

}
