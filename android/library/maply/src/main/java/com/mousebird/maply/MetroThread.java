/*  MetroThread.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/21/15.
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
import android.opengl.GLSurfaceView;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;
import android.view.Choreographer;
import java.lang.ref.WeakReference;

public class MetroThread extends HandlerThread implements Choreographer.FrameCallback
{
	public boolean requestRender = true;

	public MetroThread(String name,BaseController inControl,int inFrameInterval)
	{
		super(name);

		control = new WeakReference<>(inControl);
		frameInterval = inFrameInterval;

		start();

		// We have a looper now, but if we call postFrameCallback directly,
		// the Choreographer init throws an exception about not having a looper.
		new Handler(getLooper()).post(this::postFrameCallback);
	}

	@SuppressLint("ObsoleteSdkInt")
	public void shutdown()
	{
		removeFrameCallback();

		// Note: Is this blocking?
		try
		{
			if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.JELLY_BEAN_MR2) {
				quitSafely();
			} else {
				quit();
			}
		}
		catch (Exception e)
		{
			Log.w(Tag, "shutdown error", e);
		}
	}

	// Set the frame rate.  Will take effect on the next frame
	public void setFrameRate(int newRate) { frameInterval = newRate; }

	// Set the renderer (and scene) we'll look at render requests
	public void setRenderer(RenderController inRenderer) { renderer = inRenderer; }

	// Request a render, filtered through the regular frame rate
	public void requestRender() { requestRender = true; }

	// Called by the Choreographer to render a frame
	@Override
	public void doFrame(long frameTimeNanos)
	{
		// Nudge the renderer
		if ((frameCount % frameInterval == 0)) {
			BaseController theControl = control.get();
			android.view.View baseView = (theControl != null) ? theControl.baseView : null;
			if (baseView != null) {
				if (requestRender || (renderer != null && (renderer.hasChanges() ||
				                                           renderer.activeObjectsHaveChanges() ||
				                                           renderer.view.isAnimating()))) {
					if (baseView instanceof GLSurfaceView) {
						GLSurfaceView glSurfaceView = (GLSurfaceView)baseView;
						glSurfaceView.requestRender();
					} else if (baseView instanceof GLTextureView) {
						GLTextureView glTextureView = (GLTextureView)baseView;
						glTextureView.requestRender();
					} else {
						Log.w(Tag, String.format("Bad surface type %s", baseView.getClass().getName()));
					}
					requestRender = false;
				}
			}
		}

		// Need to do this every frame
		Choreographer.getInstance().postFrameCallback(this);

		frameCount++;
	}

	private void postFrameCallback() { Choreographer.getInstance().postFrameCallback(this); }

	private void removeFrameCallback() { Choreographer.getInstance().removeFrameCallback(this); }

	private final WeakReference<BaseController> control;
	private RenderController renderer = null;
	private int frameInterval;
	private int frameCount = 0;
	private final static String Tag = MetroThread.class.getSimpleName();
}
