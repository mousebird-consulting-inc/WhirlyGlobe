package com.mousebird.maply;

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
	
	MapView view = null;
	public void setView(MapView inView)
	{
		view = inView;
		setViewNative(inView);
	}
	
	public void doRender()
	{
		if (view != null)
			view.animate();
		
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

	public native void setScene(MapScene scene);
	public native void setViewNative(MapView view);
	public native void setClearColor(float r,float g,float b,float a);
	protected native boolean teardown();
	protected native boolean resize(int width,int height);
	protected native void render();

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
