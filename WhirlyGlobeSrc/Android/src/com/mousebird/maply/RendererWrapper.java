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

import android.opengl.GLSurfaceView;
import android.os.Build;

import java.util.concurrent.Semaphore;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * This is an internal class used to talk to the OpenGL ES surface.
 * 
 */
class RendererWrapper implements GLSurfaceView.Renderer, GLTextureView.Renderer
{
	boolean valid = true;
	public MaplyRenderer maplyRender = null;
	public Scene scene = null;
	public View view = null;
	public MaplyBaseController maplyControl = null;
	public Thread renderThread = null;
	
	public RendererWrapper(MaplyBaseController inMapControl)
	{
		maplyControl = inMapControl;
	}

	@Override
	public void finalize()
	{
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
		renderThread = Thread.currentThread();
		maplyControl.surfaceCreated(this);
	}

	public void shutdown()
	{
		maplyRender.dispose();
		maplyRender = null;
		scene = null;
		view = null;
		maplyControl = null;
		renderThread = null;
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
	boolean firstFrame = true;
	
	@Override
	public void onDrawFrame(GL10 gl)
	{
		// This is a hack to eliminate a flash we see at the beginning
		// It's a blank view on top of our view, which we get rid of when we
		//  actually draw something there.
		// http://stackoverflow.com/questions/19970829/android-and-opengl-gives-black-frame-at-startup
		if (firstFrame)
		{
			firstFrame = false;
			maplyControl.getContentView().post(
					new Runnable() {
						@Override
						public void run() {
							if (maplyControl != null && Build.VERSION.SDK_INT > 16 && Build.VERSION.SDK_INT < 24)
								maplyControl.getContentView().setBackground(null);
						}
					}
			);
		}

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
