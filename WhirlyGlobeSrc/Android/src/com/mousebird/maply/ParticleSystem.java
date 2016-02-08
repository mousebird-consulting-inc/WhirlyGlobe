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

    public ParticleSystem() {
        initialise();
    }

    public void finalize() {
        dispose();
    }

    public native void setIdent(long ident);

    public native void setName(String name);

    public native void setDrawPriority(int drawPriority);

    public native void setPointSize(float pointSize);

    public native void setParticleSystemType(int particleSystemType);

    public native void setShaderID(long shaderID);

    public native void setLifetime(double lifetime);

    public native void setBasetime(double basetime);

    public native void setBatchSize(int batchSize);

    public native void setTotalParticles(int totalParticles);

    public native void addParticleSystemAttribute(String name, int type);

    public native void addTexID(long texID);



    static {
        nativeInit();
    }

    private static native void nativeInit();
    native void initialise();
    native void dispose();
    private long nativeHandle;

}
