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

/**
 * The particle batch holds the number of particles defined in the MaplyParticleSystem batchSize property.
 * Each attribute array is added individually via an NSData object.
 * All attributes must be present or the batch is invalid and won't be passed through the system.
 */
public class ParticleBatch {

    private ParticleSystem partSys;
    private double time;

    private ParticleBatch() { }

    /**
     * Construct with the particle system this batch will be added to.
     */
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

    /**
     * Return the batch size.  This is set by the particle system.
     */
    public native int getBatchSize();

    native void addAttributeValues(float[] data);

    native void addAttributeValues(char[] data);

    /**
     * Add a float attribute by name.
     * @param attrName The name attribute.
     * @param data An array of floats that should be the length of the batch.
     */
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

    /**
     * Add a char attribute by name.
     * @param attrName The name attribute.
     * @param data An array of chars that should be the length of the batch.
     */
    public boolean addAttribute(String attrName, char [] data)
    {
        for (ParticleSystemAttribute attr : this.partSys.getAttrs()) {
            if (attrName.equals(attr.getName())) {
                // Found. Now make sure the size matches
                if (data.length != attr.getSize() * this.getBatchSize()) {
                    return false;
                }
                this.addAttributeValues(data);
                return true;
            }
        }
        return false;
    }

    /**
     * Returns truen if we've got as many attributes as we expect.
     */
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

    /**
     * Number of attributes the batch needs.
     */
    public native int getAttributesValueSize();

    public ParticleSystem getPartSys() {
        return this.partSys;
    }
}
