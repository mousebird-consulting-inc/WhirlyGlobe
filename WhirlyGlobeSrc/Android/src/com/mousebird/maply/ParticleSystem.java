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

public class ParticleSystem {

    public ParticleSystem() {
        initialise();
    }

    public void finalize() {
        dispose();
    }

    long ident = Identifiable.genID();

    /// Return a reasonable set of defaults
    public static native ParticleSystem makeDefault();

    /// Make a new randomized paticle
    public native Particle generateParticle();

    /// Starting location for particles
    public native void setLoc (Point3d loc);

    /// Randomizable particle length
    public native void setMinLength (int minLength);

    public native void setMaxLength (int maxLength);

    /// Randomizable particle lifetime in sections
    public native void setMinLifetime (float minLifetime);

    public native void setMaxLifetime (float maxLifetime);

    /// Range of the angle from the dirN to -dirN (180 total)
    public native void setMinPhi (float minPhi);

    public native void setMaxPhi (float maxPhi);

    //        Eigen::Vector3f dirN,dirE,dirUp; how?

    //        std::vector<RGBAColor> colors; util?

    public native void setMinVis (float minVis);

    public native void setMaxVis (float maxVis);

    static {
        nativeInit();
    }

    private static native void nativeInit();
    native void initialise();
    native void dispose();
}
