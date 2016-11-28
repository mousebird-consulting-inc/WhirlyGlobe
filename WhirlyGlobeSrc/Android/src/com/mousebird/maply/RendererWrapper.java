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

import android.graphics.Bitmap;
import android.opengl.GLSurfaceView;
import android.os.Build;

import java.nio.IntBuffer;
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

	private boolean doScreenshot = false;
	public MaplyBaseController.ScreenshotListener screenshotListener;



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
		try {
			renderLock.acquire();
		}
		catch (Exception e)
		{
			return;
		}

		// If the app shuts down the rendering right as the thread starts up, this can happen
		if (valid) {
			maplyRender = new MaplyRenderer();
			maplyRender.setScene(scene);
			maplyRender.setView(view);
			maplyRender.setConfig(config);
			renderThread = Thread.currentThread();
			maplyControl.surfaceCreated(this);
		}

		renderLock.release();
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
							if (maplyControl != null && Build.VERSION.SDK_INT > 16)
								if (!maplyControl.usesTextureView() || Build.VERSION.SDK_INT < 24)
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

		if (doScreenshot) {
			Bitmap screenshot = getPixels(0,0, (int)maplyControl.getViewSize().getX(), (int)maplyControl.getViewSize().getY(), gl);

			screenshotListener.onScreenshotResult(screenshot);

			screenshotListener = null;
			doScreenshot = false;
		}

		renderLock.release();
	}

	private Bitmap getPixels(int x, int y, int w, int h, GL10 gl)
	{
		int b[] = new int[w*(y+h)];
		int bt[] = new int[w*h];
		IntBuffer ib = IntBuffer.wrap(b);
		ib.position(0);
		gl.glReadPixels(x, 0, w, y+h, GL10.GL_RGBA, GL10.GL_UNSIGNED_BYTE, ib);

		for (int i=0, k=0; i<h; i++, k++) {
			// remember, that OpenGL bitmap is incompatible with Android bitmap
			// and so, some correction need.
			for (int j=0; j<w; j++) {
				int pix = b[i*w+j];
				int pb = (pix>>16) & 0xff;
				int pr = (pix<<16) & 0x00ff0000;
				int pix1 = (pix & 0xff00ff00) | pr | pb;
				bt[(h-k-1)*w+j] = pix1;
			}
		}

		return Bitmap.createBitmap(bt, w, h, Bitmap.Config.ARGB_8888);
	}

	public void takeScreenshot(MaplyBaseController.ScreenshotListener listener, GLSurfaceView surfaceView) {
		this.screenshotListener = listener;
		this.doScreenshot = true;

		if (surfaceView != null) {
			surfaceView.requestRender();
		}
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
