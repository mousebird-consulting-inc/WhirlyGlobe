/*
 *  MaplyRenderer.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2014 mousebird consulting
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

import java.util.ArrayList;
import java.util.List;

import javax.microedition.khronos.egl.*;

/**
 * The renderer encapsulates the OpenGL ES specific rendering.
 * This is opaque to the toolkit user.
 * 
 * @author sjg
 *
 */
class MaplyRenderer
{
	public Point2d frameSize = new Point2d();
	
	MaplyRenderer()
	{
		initialise();		
	}

	public void finalize()
	{
		dispose();
	}

	public boolean setup()
	{
//		return resize();
		return true;
	}
	
	public boolean surfaceChanged(int width,int height)
	{
		frameSize.setValue(width, height);
		return resize(width,height);
	}
	
	View view = null;
	public void setView(View inView)
	{
		view = inView;
		setViewNative(inView);
	}

    ArrayList<ActiveObject> activeObjects = new ArrayList<ActiveObject>();

    /**
     * Add an active object that will be called right before the render (on the render thread).
     */
    void addActiveObject(ActiveObject activeObject)
    {
        synchronized (activeObjects) {
            activeObjects.add(activeObject);
        }
    }

    /**
     * Remove an active object added earlier.
     */
    void removeActiveObject(ActiveObject activeObject)
    {
        synchronized (activeObjects) {
            activeObjects.remove(activeObject);
        }
    }

    public void doRender()
	{
		if (view != null)
			view.animate();

        // Run anyone who wants updates
        synchronized (activeObjects) {
            for (ActiveObject activeObject : activeObjects)
                activeObject.activeUpdate();
        }

		render();
	}
	
	public EGLDisplay display = null;
	public EGLConfig config = null;
	public EGLContext context = null;
	public void setConfig(EGLConfig inConfig)
	{
		config = inConfig;
		EGL10 egl = (EGL10) EGLContext.getEGL();
		display = egl.eglGetCurrentDisplay();
		context = egl.eglGetCurrentContext();
	}

	public native void setScene(Scene scene);
	public native void setViewNative(View view);
	public native void setClearColor(float r,float g,float b,float a);
	protected native boolean teardown();
	protected native boolean resize(int width,int height);
	protected native void render();
	public native void setPerfInterval(int perfInterval);
	public native void addLight(DirectionalLight light);
	public native void replaceLights(List<DirectionalLight> lights);

	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise();
	native void dispose();
	private long nativeHandle;
	
	static
	{
//		System.loadLibrary("Maply");
	}
}
