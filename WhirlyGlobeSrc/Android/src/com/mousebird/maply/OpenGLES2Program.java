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

public class OpenGLES2Program {

    long id = Identifiable.genID();

    public OpenGLES2Program(){
        initialise();
    }

    public OpenGLES2Program(String name, String vShaderString, String fShaderString){
        initialise(name, vShaderString, fShaderString);
    }

    public native boolean isValid();

    public native boolean setUniform(String name, float val);

    public native boolean setUniform(String name, Point2f vec);

    public native boolean setUniform(String name, Point3f vec);

    public native boolean setUniform(String name, Point4f vec);

    public native boolean setUniform(String name, Matrix4f mat);

    public native boolean setUniform(String name, int val);

    public native boolean setTexture (String name, int val);

    public native boolean hasLights();

    public native OpenGLESAttribute findAttribute(String attrName);

    public native String getName();

    public native int getProgram();

    public native int bindTextures();

    public native void cleanUp();

    public void finalise(){
        dispose();
    }
    native void initialise(String name, String vShaderString, String fShaderString);

    private static native void nativeInit();

    native void initialise();
    native void dispose();

    private long nativeHandle;





}
