package com.mousebird.maply;

import androidx.annotation.Keep;

/**
 * The selection manager holds the objects
 */
class SelectionManager 
{
	@Keep
	@SuppressWarnings("unused")     // Needed for JNI JavaClassInfo
	private SelectionManager() {
	}
	
	SelectionManager(Scene scene) {
		initialise(scene);
	}
	
	public void finalize() {
		dispose();
	}

	// Look for an object that the selection manager is handling
	public native long pickObject(ViewState view,Point2d screenLoc);

	// Look for a list of objects the selection manager is handling
	public SelectedObject[] pickObjects(ComponentManager compManage, ViewState view, Point2d screenLoc) {
		return pickObjects(compManage, view, screenLoc, 10.0);
	}

	public native SelectedObject[] pickObjects(ComponentManager compManage,
											   ViewState view,
											   Point2d screenLoc,
											   double maxDist);

	static {
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise(Scene scene);
	native void dispose();

	@Keep
	@SuppressWarnings("unused")     // Used by JNI
	private long nativeHandle;

	@Keep
	@SuppressWarnings("unused")     // Used by JNI
	private long nativeSceneHandle;
}
