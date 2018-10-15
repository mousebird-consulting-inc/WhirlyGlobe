/*
 *  VectorManager.java
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
 * The Vector Manager is an interface to the Maply C++ vector
 * manager and should be invisible to toolkit users.
 *
 */
class VectorManager 
{
	private VectorManager()
	{
	}
	
	VectorManager(Scene scene)
	{
		initialise(scene);
	}
	
	public void finalize()
	{
		dispose();
	}
	
	// Add vectors to the scene and return an ID to track them
	public native long addVectors(List<VectorObject> vecs,VectorInfo vecInfo,ChangeSet changes);
	
	// Remove vectors by ID
	public native void removeVectors(long ids[],ChangeSet changes);
	
	// Enable/disable vectors by ID
	public native void enableVectors(long ids[],boolean enable,ChangeSet changes);

	// Change the display of vectors
	public native void changeVectors(long ids[],VectorInfo vecInfo,ChangeSet changes);
	
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise(Scene scene);
	native void dispose();
	private long nativeHandle;
}
