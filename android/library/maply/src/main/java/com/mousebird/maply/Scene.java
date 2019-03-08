package com.mousebird.maply;

/**
 * Base class for Scene.  Use either a MapScene or a GlobeScene instead.
 * 
 * @author sjg
 *
 */
public class Scene 
{
	// Used to render individual characters using Android's Canvas/Paint/Typeface
	CharRenderer charRenderer = new CharRenderer();
	
	protected Scene()
	{
	}
	
	// Overridden by the subclass
	public void addChanges(ChangeSet changes)
	{
	}

	// Overriden by subclass
	public void shutdown()
	{
	}

	/**
	 * Associate a shader with the given scene name.  These names let us override existing shaders, as well as adding our own.
	 * @param shader The shader to add.
	 */
	public native void addShaderProgram(Shader shader);

	public native void addRenderTargetNative(long renderTargetID,int width,int height,long texID,boolean clearEveryFrame,boolean blend,float red,float green,float blue,float alpha);
	public native void removeRenderTargetNative(long renderTargetID);

	/**
	 * Tear down the OpenGL resources.  Context needs to be set first.
	 */
	public native void teardownGL();

	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void addChangesNative(ChangeSet changes);
	protected long nativeHandle;
}
