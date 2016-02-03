/*
 *  VertexAttribute.java
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


public class VertexAttribute {

    public VertexAttribute(int dataType, String name){
        initialise(dataType, name);
    }

    public VertexAttribute(VertexAttribute that){
        initialise(that);
    }
    public void finalise(){
        dispose();
    }

    public native VertexAttribute templateCopy();

    public native int getDataType();

    public native void setDefaultColor(float r, float g, float b, float a);

    public native void setDefaultVector2f(Point2f vec);

    public native void setDefaultVector3f(Point3f vec);

    public native void setDefaultFloat(float val);

    public native void addColor(float r, float g, float b, float a);

    public native void addVector2f(Point2f vec);

    public native void addVector3f(Point3f vec);

    public native void addVector4f(Point4f vec);

    public native void addFloat(float val);

    public native void addInt(int val);

    public native void reserve(int size);

    public native int numElements();

    public native void clear();

    public native Object addressForElement(int which);

    public native int glEntryComponents();

    public native int glType();

    public native boolean glNormalize();

    public native void glSetDefault(int index);

    public native void setDataType(int dataType);

    public native void setName(String name);

    public native String getName();



    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise(int dataType, String name);
    native void initialise(VertexAttribute that);
    native void dispose();
}
