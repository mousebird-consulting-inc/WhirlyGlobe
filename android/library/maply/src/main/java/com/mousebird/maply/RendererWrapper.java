/*  RendererWrapper.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2021 mousebird consulting
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
 */

package com.mousebird.maply;

import android.annotation.SuppressLint;
import android.graphics.Bitmap;
import android.opengl.GLSurfaceView;
import android.os.Build;

import java.lang.ref.WeakReference;
import java.nio.IntBuffer;
import java.util.ArrayList;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;


/**
 * This is an internal class used to talk to the OpenGL ES surface.
 * 
 */
class RendererWrapper implements GLSurfaceView.Renderer, GLTextureView.Renderer
{
	boolean valid = true;
	public Scene scene = null;
	public View view = null;
	public Thread renderThread = null;

	private boolean doScreenshot = false;
	public BaseController.ScreenshotListener screenshotListener;

	final protected WeakReference<RenderController> maplyRender;
	final protected WeakReference<BaseController> maplyControl;

	public RendererWrapper(BaseController inMapControl,RenderController inRenderControl)
	{
		maplyControl = new WeakReference<>(inMapControl);
		maplyRender = new WeakReference<>(inRenderControl);
	}

	@Override
	public void finalize()
	{
	}

	public RenderController getMaplyRender() {
		return maplyRender.get();
	}

	@Override
	public void onSurfaceCreated(GL10 gl, EGLConfig config)
	{
		try {
			if (!renderLock.tryAcquire(500, TimeUnit.MILLISECONDS))
				return;
		}
		catch (Exception e)
		{
			return;
		}

		// If the app shuts down the rendering right as the thread starts up, this can happen
		if (valid) {
			BaseController control = maplyControl.get();
			RenderController render = maplyRender.get();
			if (control != null && render != null) {
				render.setScene(scene);
				render.setView(view);
				render.setConfig(null, config);
				renderThread = Thread.currentThread();
				control.surfaceCreated(this);
			}
		}

		renderLock.release();
	}

	public void shutdown()
	{
		RenderController render = maplyRender.get();
		if (render != null) {
			render.dispose();
			maplyRender.clear();
		}
		maplyControl.clear();
		scene = null;
		view = null;
		renderThread = null;
	}
	
	@Override
	public void onSurfaceChanged(GL10 gl, int width, int height)
	{
		try {
			renderLock.acquire();
		} catch (Exception e) {
			return;
		}

		try {
			if (valid) {
				RenderController render = maplyRender.get();
				if (render != null) {
					render.surfaceChanged(width, height);
					render.doRender();
				}
			}
		} finally {
			renderLock.release();
		}
	}

	int frameCount = 0;
	final Semaphore renderLock = new Semaphore(1,true);
	boolean firstFrame = true;

	final private ArrayList<Runnable> preFrameRuns = new ArrayList<>();
	final private ArrayList<Runnable> singlePreFrameRuns = new ArrayList<>();
	final private ArrayList<Runnable> postFrameRuns = new ArrayList<>();
	final private ArrayList<Runnable> singlePostFrameRuns = new ArrayList<>();

	/**
	 * Add a runnable to the queue for pre or post frame render callbacks.
	 * If repeat is set, we'll keep it around for next frame too.
	 */
	public void addFrameRunnable(boolean preFrame,boolean repeat,Runnable run)
	{
		if (preFrame) {
			if (repeat) {
				synchronized (preFrameRuns) {
					preFrameRuns.add(run);
				}
			} else {
				synchronized (singlePreFrameRuns) {
					singlePreFrameRuns.add(run);
				}
			}
		} else {
			if (repeat) {
				synchronized (postFrameRuns) {
					postFrameRuns.add(run);
				}
			} else {
				synchronized (singlePostFrameRuns) {
					singlePostFrameRuns.add(run);
				}
			}
		}
	}

	@SuppressLint("ObsoleteSdkInt")
	@Override
	public void onDrawFrame(GL10 gl)
	{
		synchronized (singlePreFrameRuns) {
			for (Runnable run: singlePreFrameRuns)
				run.run();
			singlePreFrameRuns.clear();
		}
		synchronized (preFrameRuns) {
			for (Runnable run: preFrameRuns)
				run.run();
		}

		// This is a hack to eliminate a flash we see at the beginning
		// It's a blank view on top of our view, which we get rid of when we
		//  actually draw something there.
		// http://stackoverflow.com/questions/19970829/android-and-opengl-gives-black-frame-at-startup
		if (firstFrame && valid && maplyControl != null)
		{
			firstFrame = false;
			if (Build.VERSION.SDK_INT > 16) {
				BaseController control = maplyControl.get();
				if (control != null) {
					android.view.View contentView = control.getContentView();
					if (contentView != null) {
						contentView.post(() -> {
							if (!control.usesTextureView() || Build.VERSION.SDK_INT < 24) {
								contentView.setBackground(null);
							}
						});
					}
				}
			}
		}

		try {
			renderLock.acquire();
		} catch (Exception e) {
			return;
		}
		try {
			BaseController control = maplyControl.get();
			if (control != null) {
				RenderController render = maplyRender.get();
				if (valid && render != null) {
					render.doRender();
				}
				if (doScreenshot) {
					final int width = (int)control.getViewSize().getX();
					final int height = (int)control.getViewSize().getY();
					final Bitmap screenshot = getPixels(0, 0, width, height, gl);
					screenshotListener.onScreenshotResult(screenshot);
					screenshotListener = null;
					doScreenshot = false;
				}
			}
		} finally {
			renderLock.release();
		}

		synchronized (singlePostFrameRuns) {
			for (Runnable run: singlePostFrameRuns)
				run.run();
			singlePostFrameRuns.clear();
		}
		synchronized (postFrameRuns) {
			for (Runnable run: postFrameRuns)
				run.run();
		}
	}

	private Bitmap getPixels(int x, int y, int w, int h, GL10 gl)
	{
		int[] b = new int[w*(y+h)];
		int[] bt = new int[w*h];
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

	public void takeScreenshot(BaseController.ScreenshotListener listener, GLSurfaceView surfaceView) {
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
			if (!renderLock.tryAcquire(500, TimeUnit.MILLISECONDS)) {
				valid = false;
				return;
			}
		}
		catch (Exception e)
		{
			return;
		}
		valid = false;
		renderLock.release();
	}

}
