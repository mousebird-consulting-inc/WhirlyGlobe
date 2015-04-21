/*
 *  LayoutManager.java
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

/**
 * The layout manager interfaces to the Maply C++/JNI side of things and
 * is invisible to toolkit users.
 * 
 * @author sjg
 *
 */
class LayoutManager 
{
	private LayoutManager()
	{		
	}
	
	LayoutManager(Scene scene)
	{
		initialise(scene);
	}
	
	public void finalize()
	{
		dispose();
	}
	
	/**
	 * Set the total number of objects we'll display at once.
	 *
	 * @param numObjects Maximum number of objects to display.
	 */
	public native void setMaxDisplayObjects(int numObjects);
	
	/**
	 * Run the layout logic on the currently active objects.  Any
	 * changes will be reflected in the ChangeSet.
	 * 
	 * @param viewState View state to use for the display.
	 * @param changes Changes to propagate to the scene.
	 */
	public native void updateLayout(ViewState viewState,ChangeSet changes);
	
	/**
	 * True if there were any changes since layout was last run.
	 */
	public native boolean hasChanges();
	
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise(Scene scene);
	native void dispose();
	private long nativeHandle;
}
