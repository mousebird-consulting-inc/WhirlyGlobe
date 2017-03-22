package com.mousebird.maply;

/*
 *  VertexAttribute
 *  com.mousebirdconsulting.maply
 *
 *  Created by Steve Gifford on 4/17/16.
 *  Copyright 2013-2016 mousebird consulting
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

import android.graphics.Color;

import java.util.Set;

/**
 * A vertex attribute holds a simple data value which will be passed to a shader
 * as an attribute.  These are typically associated with individual data objects,
 * like ScreenMarkers and will be compiled together into a single attribute array
 * for use by a shader.
 * <br>
 * It's all about the Shader, basically.
 */
public class VertexAttribute
{
    private VertexAttribute()
    {
    }

    /**
     * Construct with a name.
     */
    public VertexAttribute(String name)
    {
        initialise();
        setName(name);
    }

    /**
     * Set the name of the vertex attribute.
     */
    public native void setName(String name);

    /**
     * Set the vertex attribute to the given float.
     */
    public native void setFloat(float val);

    /**
     * Set the vertex attribute to the given int.
     */
    public native void setInt(int val);

    /**
     * Set the vertex attribute to the given 2D vector.
     */
    public native void setVec2(float x,float y);

    /**
     * Set the vertex attribute to the given 3D vector.
     */
    public native void setVec3(float x,float y,float z);

    /**
     * Set the vertex attribute to the given color value.
     */
    public void setColor(int color)
    {
        setColor(Color.red(color),Color.green(color),Color.blue(color),Color.alpha(color));
    }

    /**
     * Set the vertex attribute to the given color value.
     */
    public native void setColor(int r,int g,int b,int a);

    static
    {
        nativeInit();
    }

    private static native void nativeInit();
    native void initialise();
    native void dispose();

    private long nativeHandle;
}
