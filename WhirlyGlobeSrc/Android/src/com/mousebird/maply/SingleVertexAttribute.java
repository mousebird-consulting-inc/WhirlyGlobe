/*
 *  SingleVertexAttribute.java
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

public class SingleVertexAttribute extends SingleVertexAttributeInfo{

    public SingleVertexAttribute(String name, int type){
        initialise(name, type);
    }

    public void finalise(){
        dispose();
    }

    public native void setVec4Values(float [] values);

    public native float [] getVec4Values();

    public native void setVec3Values(float [] values);

    public native float [] getVec3Values();

    public native void setVec2Values (float [] values);

    public native float [] getVec2Values();

    public native void setFloatVal(float val);

    public native float getFloatVal();

    public native void setIntVal(int val);

    public native int getIntVal();

    public native void setColor(float r, float g, float b, float a);

    public native float [] getColor();

    private static native void nativeInit();
    native void initialise(String name, int inType);
    native void dispose();
    private long nativeHandle;
}
