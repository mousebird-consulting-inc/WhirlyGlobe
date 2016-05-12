/*
 *  ParticleSystemManager.java
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


/**
 * Particle system manager controls the active particle systems
 */
public class ParticleSystemManager {

    private ParticleSystemManager() {
    }

    ParticleSystemManager(Scene scene) {
        initialize(scene);
    }

    public void finalize() {
        dispose();
    }

    // Add a group of particle systems
    public native long addParticleSystem(ParticleSystem newSystem, ChangeSet changes);

    /// Add a batch of particles
    public native void addParticleBatch(long id, ParticleBatch batch, ChangeSet changes);

    /// Enable/disable active particle system
    public native void enableParticleSystem(long sysID, boolean enable, ChangeSet changes);

    // Remove one particle system
    public native void removeParticleSystem(long sysID, ChangeSet changeSet);

    static {
        nativeInit();
    }

    private static native void nativeInit();
    native void initialize(Scene scene);
    native void dispose();
    private long nativeHandle;
}
