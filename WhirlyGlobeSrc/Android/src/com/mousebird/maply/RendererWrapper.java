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

import javax.microedition.khronos.egl.*;
import javax.microedition.khronos.opengles.*;

import android.opengl.GLSurfaceView.*;

/**
 * This is an internal class used to talk to the OpenGL ES surface.
 * 
 */
class RendererWrapper implements Renderer
{
	public MaplyRenderer maplyRender = null;
	public MapScene mapScene = null;
	public MapView mapView = null;
	public MaplyController mapControl = null;
	
	public RendererWrapper(MaplyController inMapControl)
	{
		mapControl = inMapControl;
	}
	
	@Override
	public void onSurfaceCreated(GL10 gl, EGLConfig config)
	{		
		maplyRender = new MaplyRenderer();
  		maplyRender.setScene(mapScene);
		maplyRender.setView(mapView);
		maplyRender.setConfig(config);
		mapControl.surfaceCreated(this);
	}
	
	@Override
	public void onSurfaceChanged(GL10 gl, int width, int height)
	{
		maplyRender.surfaceChanged(width,height);
		maplyRender.doRender();
	}
	
	@Override
	public void onDrawFrame(GL10 gl)
	{
		try {
			// Nudge this past the 1/60s frame boundary
			// Note: Porting.  Egregious hack.
			Thread.sleep(16);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		maplyRender.doRender();		
	}
}
