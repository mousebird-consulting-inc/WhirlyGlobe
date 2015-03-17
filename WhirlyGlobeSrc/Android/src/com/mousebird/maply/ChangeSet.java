/*
 *  ChangeSet.java
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
 * The change set is a largely opaque object the Maply uses to
 * track visual changes in the map or globe.  Most of the action
 * takes place behind the scenes and users of the Maply API should
 * not be manipulating these.
 * 
 */
class ChangeSet
{
	ChangeSet()
	{
		initialise();
	}
	
	// Add a texture to the list of changes to the scene
	public native void addTexture(Texture texture);
	
	// Remove a texture from the scene by ID
	public native void removeTexture(long texID);
	
	// Merge a new set of changes in at the end
	// This clears the changes in the ChangeSet passed in
	public native void merge(ChangeSet changes);
	
	// Create whatever objects want to be created.
	// We're assuming a valid EGL context is in place
	public native void process(Scene scene);
	
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	public void finalize()
	{
		dispose();
	}
	native void initialise();
	native void dispose();	
	private long nativeHandle;	
}
