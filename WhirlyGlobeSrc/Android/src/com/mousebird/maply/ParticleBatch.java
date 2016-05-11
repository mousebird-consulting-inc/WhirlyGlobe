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

/**
 * A particle batch adds a set number of particles to the system.
 * <br>
 * The particle batch holds the number of particles defined in the
 * MaplyParticleSystem batchSize property.
 * Each attribute array is added individually via an NSData object.
 * All attributes must be present or the batch is invalid and won't
 * be passed through the system.
 */
public class ParticleBatch {

    private ParticleSystem partSys;
    private double time;

    private ParticleBatch() {
    }

	/**
     * Initialize with the particle system.
     * The batch is initialized with its particle system.
     * You must then call addAttributeValues() repeatedly with attribute arrays.
     * @param partSys The particle system this batch belongs to.
     */
    public ParticleBatch(ParticleSystem partSys) {
        initialise();
        this.partSys = partSys;
        this.setBatchSize(this.partSys.getBatchSize());
    }

    public void finalize() {
        dispose();
    }

	/**
     * Batch size for MaplyParticleBatch.
     * Should match the particle systems batch size
     * @param batchSize Batch size for MaplyParticleBatch.
     */
    private native void setBatchSize(int batchSize);

	/**
     * @return Batch size for MaplyParticleBatch.
     */
    public native int getBatchSize();

    public native void addAttributeValues(float[] data);
    public native void addAttributeValues(char[] data);

    /**
     * Add an attribute array of the given name.
     * <br>
     * Each attribute in the MaplyParticleSystem must be filled in here.
     * The name must correspond and the length of the data must match.
     * @return true if the attribute array was valid, false otherwise.
     */
    public boolean addAttribute(String attrName, float[] data) {
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
     * Add an attribute array of the given name.
     * <br>
     * Each attribute in the MaplyParticleSystem must be filled in here.
     * The name must correspond and the length of the data must match.
     * @return true if the attribute array was valid, false otherwise.
     */
    public boolean addAttribute(String attrName, char[] data)
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
     * Tests if the batch is valid.
     * <br>
     * This checks if all the attribute arrays are present and valid.
     * @return if the batch is valid.
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

    public native int getAttributesValueSize();

	/**
     * @return The particle system this batch belongs to.
     */
    public ParticleSystem getPartSys() {
        return this.partSys;
    }
}
