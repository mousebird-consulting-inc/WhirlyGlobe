/*
 *  MetroThread.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/21/15.
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

import android.os.Handler;
import android.os.HandlerThread;
import android.view.Choreographer;

public class MetroThread extends HandlerThread implements Choreographer.FrameCallback
{
	Choreographer ch;
	MaplyBaseController control;
	int frameInterval;
	
	public MetroThread(String name,MaplyBaseController inControl,int inFrameInterval) 
	{
		super(name);

		control = inControl;
		frameInterval = inFrameInterval;
		
		start();
		final MetroThread metThread = this;
		Handler handler = new Handler(this.getLooper());
		handler.post(new Runnable()
		{
			@Override
			public void run()
			{
				Choreographer.getInstance().postFrameCallback(metThread);
			}
		});
	}
	
	public void shutdown()
	{
		// Note: Is this blocking?
		try
		{
			quit();
		}
		catch (Exception e)
		{	
		}		
	}
	
	int frameCount = 0;

	// Called by the Choreographer to render a frame
	@Override
	public void doFrame(long frameTimeNanos) 
	{		
		// Nudge the renderer
		if (control.glSurfaceView != null && (frameCount % frameInterval == 0))
			control.glSurfaceView.requestRender();
		
		// Need to do this every frame
    	Choreographer.getInstance().postFrameCallback(this);
    	
    	frameCount++;
	}
}
