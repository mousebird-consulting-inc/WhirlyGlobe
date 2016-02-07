/*
 *  SingleVertexAttributeInfo.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
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

public class SingleVertexAttributeInfo {

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

    public SingleVertexAttributeInfo() {
        initialise();
    }

    public void finalize() {
        dispose();
    }

    long ident = Identifiable.genID();

    public native void setName(String name);

    public native void setType (int type);

    public MaplyShaderAttrType getTypeEnum(){
        switch (this.getType()){
            case 0:
                return MaplyShaderAttrType.MAPLY_SHADER_ATTR_TYPE_INT;
            case 1:
                return MaplyShaderAttrType.MAPLY_SHADER_ATTR_TYPE_FLOAT;
            case 2:
                return MaplyShaderAttrType.MAPLY_SHADER_ATTR_TYPE_FLOAT2;
            case 3:
                return MaplyShaderAttrType.MAPLY_SHADER_ATTR_TYPE_FLOAT3;
            case 4:
                return MaplyShaderAttrType.MAPLY_SHADER_ATTR_TYPE_FLOAT4;
            default:
                return MaplyShaderAttrType.MAPLY_SHADER_ATTR_TYPE_INT;

        }
    }

    public int getTypeSize(){

        int size = 0;
        switch (this.getTypeEnum()){
            case MAPLY_SHADER_ATTR_TYPE_INT:
                size = 4;
                break;
            case MAPLY_SHADER_ATTR_TYPE_FLOAT:
                size = 4;
                break;
            case MAPLY_SHADER_ATTR_TYPE_FLOAT2:
                size = 2*4;
                break;
            case MAPLY_SHADER_ATTR_TYPE_FLOAT3:
                size = 3*4;
                break;
            case MAPLY_SHADER_ATTR_TYPE_FLOAT4:
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
