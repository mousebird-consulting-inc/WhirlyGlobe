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

	/**
	 * Associate a shader with the given scene name.  These names let us override existing shaders, as well as adding our own.
	 * @param shader The shader to add.
	 * @param sceneName The scene name to associate it with.
	 */
	public native void addShaderProgram(Shader shader,String sceneName);

	/**
	 * Tear down the OpenGL resources.  Context needs to be set first.
	 */
	public native void teardownGL();

	public native long getProgramIDBySceneName(String shaderName);

	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void addChangesNative(ChangeSet changes);
	protected long nativeHandle;
}
