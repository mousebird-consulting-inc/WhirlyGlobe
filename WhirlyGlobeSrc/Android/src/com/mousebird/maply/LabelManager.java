/*
 *  LabelManager.java
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
 * The label manager interfaces with the C++ side label manager for.. managing labels.
 * This isn't something you need to be interacting with.
 */
class LabelManager 
{
	private LabelManager()
	{
	}
	
	LabelManager(Scene scene)
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
	native void initialise(Scene scene);
	native void dispose();
	private long nativeHandle;
	private long nativeSceneHandle;
}
