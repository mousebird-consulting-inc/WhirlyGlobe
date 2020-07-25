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

import android.os.Looper;
import android.util.Log;

import java.lang.ref.WeakReference;
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
	WeakReference<RenderControllerInterface> control;

	// Shaders can be looked up by name
	public static final String DefaultTriangleShader = "Default Triangle;lighting=yes";
	public static final String NoLightTriangleShader = "Default Triangle;lighting=no";
	public static final String DefaultLineShader = "Default Line;backface=yes";
	public static final String NoBackfaceLineShader = "Default Line;backface=no";
	public static final String DefaultModelTriShader = "Default Triangle;model=yes;lighting=yes";
	public static final String DefaultTriScreenTexShader = "Default Triangle;screentex=yes;lighting=yes";
	public static final String DefaultTriMultiTexShader = "Default Triangle;multitex=yes;lighting=yes";
	public static final String DefaultTriMultiTexRampShader = "Default Triangle;multitex=yes;lighting=yes;ramp=yes";
	public static final String DefaultMarkerShader = "Default marker;multitex=yes;lighting=yes";
	public static final String DefaultTriNightDayShader = "Default Triangle;nightday=yes;multitex=yes;lighting=yes";
	public static final String BillboardGroundShader = "Default Billboard ground";
	public static final String BillboardEyeShader = "Default Billboard eye";
	public static final String DefaultWideVectorShader = "Default Wide Vector";
	public static final String DefaultWideVectorGlobeShader = "Default Wide Vector Globe";
	public static final String DefaultScreenSpaceMotionShader = "Default Screenspace Motion";
	public static final String DefaultScreenSpaceShader = "Default Screenspace";

	// Types used to describe the shader attributes
	enum AttributeType {
		Int,
		Float,
		Float2,
		Float3,
		Float4
	};

	/** Initialize with the file names for the shader program.
	 * <p>
     * See initWithName:vertex:fragment:viewC: for more details on how this works.
     * @param name The name of the shader program.  Used for identification and sometimes lookup.
     * @param vertexSrc The string containing the full vertex program.
     * @param fragSrc The string containing the full fragment program.
     * @param inControl The controller where we'll register the new shader.
     * @return Returns a shader program if it succeeded.  It may not work, however, so call valid first.
     */
	public Shader(String name,String vertexSrc, String fragSrc,RenderControllerInterface inControl)
	{
        control = new WeakReference<RenderControllerInterface>(inControl);
		RenderControllerInterface.ContextInfo context = control.get().setupTempContext(RenderController.ThreadMode.ThreadCurrent);

		initialise(name,vertexSrc,fragSrc);

		if (context != null)
			control.get().clearTempContext(context);
	}

	/**
	 * Initialise with an empty program we'll fill in later.
	 *
	 * @param inControl The control we'll associate this program with.
	 */
	public Shader(RenderControllerInterface inControl)
	{
		control = new WeakReference<RenderControllerInterface>(inControl);

		RenderControllerInterface.ContextInfo context = control.get().setupTempContext(RenderController.ThreadMode.ThreadCurrent);
		if (context == null) {
			Log.i("Maply","Shader was set up before context was created.  Shader won't work.");
			return;
		}

		initialise();

		control.get().clearTempContext(context);
	}

	/**
	 * This is called after an a minimal setup.  Presumably you'll be setting other attributes
	 * (like varying names) which must be passed in when the program is created.
	 *
	 * @param name The name of the shader program.  Used for identification and sometimes lookup.
	 * @param vertexSrc The string containing the full vertex program.
	 * @param fragSrc The string containing the full fragment program.
	 */
	public void delayedSetup(String name,String vertexSrc,String fragSrc)
	{
		RenderControllerInterface theControl = control.get();
		if (theControl == null)
			return;

		RenderControllerInterface.ContextInfo context = theControl.setupTempContext(RenderController.ThreadMode.ThreadCurrent);
		if (context == null) {
			Log.i("Maply","Shader was set up before context was created.  Shader won't work.");
			return;
		}

		delayedSetupNative(name,vertexSrc,fragSrc);

		theControl.clearTempContext(context);
	}

    protected Shader()
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
	 * @param texture Texture to pass into the shader.
	 */
	public void addTexture(String name,MaplyTexture texture)
	{
		ChangeSet changes = new ChangeSet();

		textures.add(texture);
		addTextureNative(changes,name,texture.texID);

//        Log.d("Maply","addTexture texID " + texture.texID);

		control.get().processChangeSet(changes);
	}

	native void addTextureNative(ChangeSet changes,String name,long texID);
	
	/** Set a float uniform in the shader with the given name.
	 * <p>
     * @return Returns true if there was such a uniform, false otherwise.
	 */
	public boolean setUniform(String name,double uni)
	{
		RenderControllerInterface.ContextInfo context = control.get().setupTempContext(RenderController.ThreadMode.ThreadCurrent);

		control.get().requestRender();

		boolean ret = setUniformNative(name,uni);
		control.get().clearTempContext(context);

		return ret;
	}

	/**
	 * Set a float uniform in the shader with a given name at the given index.
	 * Specifically for arrays.
	 */
	public boolean setUniformByIndex(String name,double uni,int index)
	{
		RenderControllerInterface.ContextInfo context = control.get().setupTempContext(RenderController.ThreadMode.ThreadCurrent);

		control.get().requestRender();

		boolean ret = setUniformByIndexNative(name,uni,index);
		control.get().clearTempContext(context);

		return ret;
	}

	public native boolean setUniformNative(String name,double uni);
	public native boolean setUniformByIndexNative(String name,double uni,int index);

	/** Set an int uniform in the shader with the given name.
	 * <p>
     * @return Returns true if there was such a uniform, false otherwise.
	 */
	public boolean setUniform(String name,int uni)
	{
		RenderControllerInterface.ContextInfo context = control.get().setupTempContext(RenderController.ThreadMode.ThreadCurrent);

		control.get().requestRender();

		boolean ret = setUniformNative(name,uni);
		control.get().clearTempContext(context);

		return ret;
	}

	public native boolean setUniformNative(String name,int uni);

	/**
	 * Set a pair of doubles in the shader with the given name.
	 * <p>
	 * @param pt Point to set the uniform to.
     * @return Returns true if there was such a uniform, false otherwise.
	 */
	public boolean setUniform(String name,Point2d pt)
	{
		RenderControllerInterface.ContextInfo context = control.get().setupTempContext(RenderController.ThreadMode.ThreadCurrent);

		control.get().requestRender();

		boolean ret = setUniformNative(name,pt.getX(),pt.getY());
		control.get().clearTempContext(context);

		return ret;
	}

	/**
	 * Set a pair of doubles in the shader with the given name.
	 * <p>
	 * @param pt Point to set the uniform to.
     * @return Returns true if there was such a uniform, false otherwise.
	 */
	public boolean setUniform(String name,Point3d pt)
	{
		RenderControllerInterface.ContextInfo context = control.get().setupTempContext(RenderController.ThreadMode.ThreadCurrent);

		control.get().requestRender();

		boolean ret = setUniformNative(name, pt.getX(), pt.getY(), pt.getZ());
		control.get().clearTempContext(context);

		return ret;
	}

	/**
	 * Set a pair of doubles in the shader with the given name.
	 * <p>
	 * @param pt Point to set the uniform to.
     * @return Returns true if there was such a uniform, false otherwise.
	 */
	public boolean setUniform(String name,Point4d pt)
	{
		RenderControllerInterface.ContextInfo context = control.get().setupTempContext(RenderController.ThreadMode.ThreadCurrent);

		control.get().requestRender();

		boolean ret = setUniformNative(name, pt.getX(), pt.getY(), pt.getZ(), pt.getW());
		control.get().clearTempContext(context);

		return ret;
	}

	/**
	 * Set the 4 component color value for a uniform with the given index (e.g. it's an array)
	 */
	public boolean setUniformColorByIndex(String name,int color,int index)
	{
		RenderControllerInterface.ContextInfo context = control.get().setupTempContext(RenderController.ThreadMode.ThreadCurrent);

		control.get().requestRender();

		boolean ret = setUniformColorByIndexNative(name, color, index);
		control.get().clearTempContext(context);

		return ret;
	}

	/**
	 * Set the 4 component color value for a uniform.
	 */
	public boolean setUniformColor(String name,int color)
	{
		RenderControllerInterface.ContextInfo context = control.get().setupTempContext(RenderController.ThreadMode.ThreadCurrent);

		control.get().requestRender();

		boolean ret = setUniformColorNative(name,color);
		control.get().clearTempContext(context);

		return ret;
	}

	native boolean setUniformNative(String name,double uniX,double uniY);
	native boolean setUniformNative(String name,double uniX,double uniY,double uniZ);
	native boolean setUniformNative(String name,double uniX,double uniY,double uniZ,double uniW);
	native boolean setUniformColorNative(String name,int color);
	native boolean setUniformColorByIndexNative(String name,int color,int index);

	/**
	 * Varyings will be passed from one shader to another using transform feedback.
	 * It's weird and it's annoying and it has to be done at Shader setup.
	 *
	 * @param name Name of the output of the vertex stage to turn into a varying.
	 */
	public native void addVarying(String name);

	/**
	 * Returns the internal Maply ID for the shader.
     */
	public native long getID();

	static
	{
		nativeInit();
	}
	public void finalize()
	{
		dispose();
	}
	private static native void nativeInit();
	native void initialise(String name,String vertexSrc, String fragSrc);
	native void initialise();
	native void delayedSetupNative(String name,String vertexSrc,String fragSrc);
	native void dispose();
	private long nativeHandle;
	private long nativeSceneHandle;
}
