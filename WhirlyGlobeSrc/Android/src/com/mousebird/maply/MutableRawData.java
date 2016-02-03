/*
 *  OpenGLES2Program.java
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

public class MutableRawData extends RawData{

    public MutableRawData(){
        initialise();
    }

    public MutableRawData (int size){
        initialise(size);
    }

    public native char getRawData();

    public native long getLen();

    public native void addInt(int iVal);

    public native void addDouble(double dVal);

    public native void addString(String str);

    public void finalise(){
        dispose();
    }


    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void initialise(int size);
    native void dispose();
}
