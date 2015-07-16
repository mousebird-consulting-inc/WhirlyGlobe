/*
 *  MapScene.java
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
 * The map scene represents an internal Maply Scene object and
 * is completely opaque to toolkit users.
 * 
 * @author sjg
 *
 */
public class MapScene extends Scene
{
	private MapScene()
	{
	}
	
	MapScene(CoordSystemDisplayAdapter coordAdapter)
	{
		initialise(coordAdapter,charRenderer);
	}
	
	public void finalize()
	{
		dispose();
	}
	
	// Flush the given changes out to the Scene
	@Override public void addChanges(ChangeSet changes)
	{
		addChangesNative(changes);
	}

	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise(CoordSystemDisplayAdapter coordAdapter,CharRenderer charRenderer);
	native void dispose();
}
