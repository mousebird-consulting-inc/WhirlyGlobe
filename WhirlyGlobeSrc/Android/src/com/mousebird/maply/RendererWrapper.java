package com.mousebird.maply;

import javax.microedition.khronos.egl.*;
import javax.microedition.khronos.opengles.*;

import android.opengl.GLSurfaceView.*;

/**
 * This is an internal class used to talk to the OpenGL ES surface.
 * 
 * @author sjg
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
		maplyRender.doRender();		
	}
}
