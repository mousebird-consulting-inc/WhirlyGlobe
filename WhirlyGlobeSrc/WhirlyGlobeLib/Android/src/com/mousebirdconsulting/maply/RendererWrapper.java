package com.mousebirdconsulting.maply;

import javax.microedition.khronos.opengles.GL10;

import android.opengl.GLSurfaceView.*;
import javax.microedition.khronos.egl.EGLConfig;

class RendererWrapper implements Renderer
{
	public MaplyRenderer maplyRender;
	public MapScene mapScene;
	public MapView mapView;
	
	public RendererWrapper()
	{
	}
	
	@Override
	public void onSurfaceCreated(GL10 gl, EGLConfig config)
	{		
//		GLES20.glClearColor(1.0f,0.0f,1.0f,0.0f);
		
		maplyRender = new MaplyRenderer();
  		maplyRender.setScene(mapScene);
		maplyRender.setView(mapView);
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
//		GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
		
		maplyRender.doRender();
	}
}
