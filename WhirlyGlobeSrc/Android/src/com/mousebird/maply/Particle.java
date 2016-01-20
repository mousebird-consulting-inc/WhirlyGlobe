/*
 *  Particle.java
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

public class Particle {

    public Particle() {
        initialise();
    }

    public void finalize() {
        dispose();
    }

    /// Location, which is updated every frame
    public native void setLoc (Point3d loc);

    /// Particle color
    public native void setColor(float r,float g,float b,float a);

    /// Particle velocity
    public native void setVelocity (float velocity);

    //        Eigen::Vector3f dir; how?

    /// When this particle is done
    public native void setExpiration (double expiration);


    static {
        nativeInit();
    }

    private static native void nativeInit();
    native void initialise();
    native void dispose();
}
