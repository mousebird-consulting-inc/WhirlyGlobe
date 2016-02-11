/*
 *  ParticleBatch.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro on 21/1/16.
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

public class ParticleBatch {

    private ParticleSystem partSys;
    private double time;

    public ParticleBatch(ParticleSystem partSys)
    {
        initialise();
        this.partSys = partSys;
        this.setBatchSize(this.partSys.getBatchSize());
    }

    public void finalize() {
        dispose();
    }

    private native void setBatchSize(int batchSize);

    public native int getBatchSize();

    public native void addAttributeValues(float[] data);

    public boolean addAttribute(String attrName, float [] data) {
        for (ParticleSystemAttribute attr : this.partSys.getAttrs()) {
            if (attrName.equals(attr.getName())) {
                // Found. Now make sure the size matches
                if (data.length * (Float.SIZE / 8) != attr.getSize() * this.getBatchSize()) {
                    return false;
                }
                this.addAttributeValues(data);
                return true;
            }
        }
        return false;
    }

    public boolean isValid() {
        return this.partSys.getAttrs().length == this.getAttributesValueSize();
    }
    static {
        nativeInit();
    }

    private static native void nativeInit();
    native void initialise();
    native void dispose();
    private long nativeHandle;

    public native int getAttributesValueSize();

    public ParticleSystem getPartSys() {
        return this.partSys;
    }
}
