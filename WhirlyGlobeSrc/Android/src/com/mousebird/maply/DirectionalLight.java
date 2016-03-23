/*
 *  DirectionalLight.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2016 mousebird consulting
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


public class DirectionalLight {

    public DirectionalLight() {
        initialise();
    }

    public void finalise() {
        dispose();
    }

    public native void setPos(Point3d pos);

    public native Point3d getPos();

    public native void setViewDependent(boolean viewDependent);

    public native boolean getViewDependent();

    public native void setAmbient(Point4d ambient);

    public native Point4d getAmbient();

    public native void setDiffuse(Point4d diffuse);

    public native Point4d getDifusse();

    public native void setSpecular(Point4d specular);

    public native Point4d getSpecular();

    public native boolean bindToProgram(Shader shader, int index, Matrix4d modelMat);

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();
    private long nativeHandle;
}
