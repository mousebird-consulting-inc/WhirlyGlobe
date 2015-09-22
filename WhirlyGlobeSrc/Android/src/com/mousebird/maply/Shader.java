/*
 *  Shader.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 8/26/15.
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

import android.util.Log;

import java.util.ArrayList;

/**
 * The shader is a direct interface to OpenGL ES 2.0 shader language.
 * <p>
 * You can set your own shader programs in the toolkit!  Yeah, that's as complex as it sounds.
 * The underyling toolkit makes a distinction between the name of the shader and the scene name.  The scene name is used as a way to replace the default shaders we use for triangles and lines.  This would let you replace the shaders you're already using with your own.  See the addShaderProgram:sceneName: method in the MaplyBaseViewController.
 * You can also add your own shader and hook it up to any features that can call out a specific shader, such as the MaplyQuadImageTilesLayer.
 * When writing a new shader, go take a look at DefaultShaderPrograms.mm, particularly the vertexShaderTri and fragmentShaderTri.  The documentation here is for the uniforms and attributes the system is going to hook up for you.  All of these are optional, but obviously nothing much will happen if you don't use the vertices.
 * 
 */
public class Shader 
{
	MaplyBaseController control = null;

	/** Initialize with the file names for the shader program.
	 * <p>
     * See initWithName:vertex:fragment:viewC: for more details on how this works.
     * @param name The name of the shader program.  Used for identification and sometimes lookup.
     * @param vertexSrc The string containing the full vertex program.
     * @param fragSrc The string containing the full fragment program.
     * @param control The controller where we'll register the new shader.
     * @return Returns a shader program if it succeeded.  It may not work, however, so call valid first.
     */
	public Shader(String name,String vertexSrc, String fragSrc,MaplyBaseController inControl)
	{
        control = inControl;
		if (control.setEGLContext())
			initialise(name,vertexSrc,fragSrc);
		else
			Log.i("Maply","Shader was set up before context was created.  Shader won't work.");
	}

    private Shader()
    {
    }
	
	/** Check if the shader is valid.
	 * <p>
     * The shader setup can fail in a number of ways.  Check this after creating the shader to see if it succeeded.  If not, look to getError to see why.
	 */
	public native boolean valid();

    /**
     * Returns the shader's name.
     */
	public native String getName();

	// Textures attached to this shader
	ArrayList<MaplyTexture> textures = new ArrayList<MaplyTexture>();

	/**
	 * Add a texture for use in the shader.
	 * @param name Name to be used in the shader.
	 * @param bitmap Bitmap to pass into the shader.
	 */
	public void addTexture(String name,MaplyTexture texture)
	{
		textures.add(texture);

		addTextureNative(control.getScene(),name,texture.texID);
	}

	native void addTextureNative(Scene scene,String name,long texID);
	
	/** Set a float uniform in the shader with the given name.
	 * <p>
     * @return Returns true if there was such a uniform, false otherwise.
	 */
	public native boolean setUniform(String name,double uni);

	/** Set an int uniform in the shader with the given name.
	 * <p>
     * @return Returns true if there was such a uniform, false otherwise.
	 */
	public native boolean setUniform(String name,int uni);

	/**
	 * Set a pair of doubles in the shader with the given name.
	 * <p>
	 * @param pt Point to set the uniform to.
     * @return Returns true if there was such a uniform, false otherwise.
	 */
	public boolean setUniform(String name,Point2d pt)
	{
		return setUniform(name,pt.getX(),pt.getY());
	}

	/**
	 * Set a pair of doubles in the shader with the given name.
	 * <p>
	 * @param pt Point to set the uniform to.
     * @return Returns true if there was such a uniform, false otherwise.
	 */
	public boolean setUniform(String name,Point3d pt)
	{
		return setUniform(name,pt.getX(),pt.getY(),pt.getZ());
	}

	/**
	 * Set a pair of doubles in the shader with the given name.
	 * <p>
	 * @param pt Point to set the uniform to.
     * @return Returns true if there was such a uniform, false otherwise.
	 */
	public boolean setUniform(String name,Point4d pt)
	{
		return setUniform(name,pt.getX(),pt.getY(),pt.getZ(),pt.getW());
	}

	native boolean setUniform(String name,double uniX,double uniY);
	native boolean setUniform(String name,double uniX,double uniY,double uniZ);
	native boolean setUniform(String name,double uniX,double uniY,double uniZ,double uniW);
	
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise(String name,String vertexSrc, String fragSrc);
	native void dispose();
	private long nativeHandle;
	private long nativeSceneHandle;
}
