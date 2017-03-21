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

/**
 * This implements a simple directional light source
 * This is an internal class for interacing to the C++ side.
 */
class DirectionalLight {

    public DirectionalLight() {
        initialise();
    }

    public void finalize() {
        dispose();
    }

    /**
     * @param pos Light position
     */
    public native void setPos(Point3d pos);

    /**
     * @return Light position
     */
    public native Point3d getPos();

    /**
     * @param viewDependent If set, we won't process the light position through the model matrix
     */
    public native void setViewDependent(boolean viewDependent);

    /**
     * @return If set, we won't process the light position through the model matrix
     */
    public native boolean getViewDependent();

    /**
     * @param ambient Ambient light color
     */
    public native void setAmbient(Point4d ambient);

    /**
     * @return Ambient light color
     */
    public native Point4d getAmbient();

    /**
     * @param diffuse Diffuse light color
     */
    public native void setDiffuse(Point4d diffuse);

    /**
     * @return Diffuse light color
     */
    public native Point4d getDifusse();

    /**
     * @param specular Specular light color
     */
    public native void setSpecular(Point4d specular);

    /**
     * @return Specular light color
     */
    public native Point4d getSpecular();

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();
    private long nativeHandle;
}
