/*
 *  RendererWrapper.java
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

import android.opengl.GLSurfaceView.Renderer;

import java.util.concurrent.Semaphore;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * This is an internal class used to talk to the OpenGL ES surface.
 * 
 */
class RendererWrapper implements Renderer
{
	boolean valid = true;
	public MaplyRenderer maplyRender = null;
	public Scene scene = null;
	public View view = null;
	public MaplyBaseController maplyControl = null;
	
	public RendererWrapper(MaplyBaseController inMapControl)
	{
		maplyControl = inMapControl;
	}

	public MaplyRenderer getMaplyRender() {
		return maplyRender;
	}

	@Override
	public void onSurfaceCreated(GL10 gl, EGLConfig config)
	{		
		maplyRender = new MaplyRenderer();
  		maplyRender.setScene(scene);
		maplyRender.setView(view);
		maplyRender.setConfig(config);
		maplyControl.surfaceCreated(this);
	}
	
	@Override
	public void onSurfaceChanged(GL10 gl, int width, int height)
	{
		try {
			renderLock.acquire();
		}
		catch (Exception e)
		{
			return;
		}

		if (valid) {
			maplyRender.surfaceChanged(width, height);
			maplyRender.doRender();
		}

		renderLock.release();
	}

	int frameCount = 0;
	Semaphore renderLock = new Semaphore(1,true);
	
	@Override
	public void onDrawFrame(GL10 gl)
	{
		try {
			renderLock.acquire();
		}
		catch (Exception e)
		{
			return;
		}

		if (valid)
			maplyRender.doRender();

		renderLock.release();
	}

	/**
	 * Blocks until the rendering is over, then no more rendering.
	 */
	public void stopRendering()
	{
		try {
			renderLock.acquire();
		}
		catch (Exception e)
		{
			return;
		}
		valid = false;
		renderLock.release();
	}

}
