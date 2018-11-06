/*
 *  MarkerManager.java
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

import java.util.List;

/**
 * The marker manager interfaces to the Maply C++/JNI side of things
 * and is invisible to toolkit users.
 *
 */
class MarkerManager 
{
	private MarkerManager()
	{		
	}
	
	MarkerManager(Scene scene)
	{
		initialise(scene);
	}
	
	public void finalize()
	{
		dispose();
	}

	// Add markers to the scene and return an ID to track them
	public native long addScreenMarkers(List<InternalMarker> markers,MarkerInfo markerInfo,ChangeSet changes);

	// Add markers to the scene and return an ID to track them
	public native long addMarkers(List<InternalMarker> markers,MarkerInfo markerInfo,ChangeSet changes);

	// Remove markers by ID
	public native void removeMarkers(long ids[],ChangeSet changes);
	
	// Enable/disable markers by ID
	public native void enableMarkers(long ids[],boolean enable,ChangeSet changes);
	
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise(Scene scene);
	native void dispose();
	private long nativeHandle;
	private long nativeSceneHandle;
}
