/*
 *  ParticleSystemAttribute.java
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

public class ParticleSystemAttribute {

    public enum MaplyShaderAttrType {
        MAPLY_SHADER_ATTR_TYPE_INT(0),
        MAPLY_SHADER_ATTR_TYPE_FLOAT(1),
        MAPLY_SHADER_ATTR_TYPE_FLOAT2(2),
        MAPLY_SHADER_ATTR_TYPE_FLOAT3(3),
        MAPLY_SHADER_ATTR_TYPE_FLOAT4(4);

        private final int value;

        MaplyShaderAttrType(int value) {
            this.value = value;
        }

        public int getValue() {
            return this.value;
        }

    }

    public ParticleSystemAttribute() {
        initialise();
    }

    public void finalize() {
        dispose();
    }

    long ident = Identifiable.genID();

    public native void setName(String name);

    public native void setType (int type);

    public int getTypeSize(){

        int size = 0;
        switch (this.getType()){
            case 0:
                size = 4;
                break;
            case 1:
                size = 4;
                break;
            case 2:
                size = 2*4;
                break;
            case 3:
                size = 3*4;
                break;
            case 4:
                size = 4*4;
                break;
            default:
                size = 0;
                break;
        }
        return size;
    }

    private native int getType();

    public native String getName();

    static {
        nativeInit();
    }

    private static native void nativeInit();
    native void initialise();
    native void dispose();
    private long nativeHandle;

}
