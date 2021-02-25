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

    public ParticleSystem partSys;

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
        this.setPartSysNative(this.partSys);
    }

    public void finalize() {
        dispose();
    }

    private native void setPartSysNative(ParticleSystem partSys);

    /**
     * This will be set by default.  However, you can control what the time basis for a particle batch is.
     */
    public native void setTime(double time);

    /**
     * The time basis is used to offset times passed into the particle batch.
     */
    public native double getTime();

    /**
     * Add an attribute array of the given name.
     * <br>
     * Each attribute in the MaplyParticleSystem must be filled in here.
     * The name must correspond and the length of the data must match.
     * @return true if the attribute array was valid, false otherwise.
     */
    public native boolean addAttribute(String name,float[] data);

    /**
     * Add an attribute array of the given name.
     * <br>
     * Each attribute in the MaplyParticleSystem must be filled in here.
     * The name must correspond and the length of the data must match.
     * @return true if the attribute array was valid, false otherwise.
     */
    public native boolean addAttribute(String name,char[] data);

	/**
     * Tests if the batch is valid.
     * <br>
     * This checks if all the attribute arrays are present and valid.
     * @return if the batch is valid.
     */
    public native boolean isValid();

    static {
        nativeInit();
    }

    private static native void nativeInit();
    native void initialise();
    native void dispose();
    private long nativeHandle;
}
