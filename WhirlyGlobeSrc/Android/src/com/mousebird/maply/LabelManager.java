package com.mousebird.maply;

import java.util.List;

class LabelManager 
{
	private LabelManager()
	{
	}
	
	LabelManager(MapScene scene)
	{
		initialise(scene);
	}
	
	public void vinalize()
	{
		dispose();
	}
	
	// Add labels to the scene and return an ID to track them
	public native long addLabels(List<InternalLabel> labels,LabelInfo labelInfo,ChangeSet changes);
	
	// Remove labels by ID
	public native void removeLabels(long ids[],ChangeSet changes);
	
	// Enable/disable labels by ID
	public native void enableLabels(long ids[],boolean enable,ChangeSet changes);

	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise(MapScene scene);
	native void dispose();
	private long nativeHandle;
	private long nativeSceneHandle;
}
