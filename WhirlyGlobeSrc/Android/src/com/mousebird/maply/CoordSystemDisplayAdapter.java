/*
 *  CoordSystemDisplayAdapter.java
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
 * The coordinate system display adapter controls the conversion from source
 * data coordinates to display coordinates.  It's completely opaque outside
 * of the toolkit.
 *
 */
class CoordSystemDisplayAdapter 
{
	CoordSystem coordSys = null;
	
	// Needed by the JNI side
	protected CoordSystemDisplayAdapter()
	{		
	}
	
	CoordSystemDisplayAdapter(CoordSystem inCoordSys)
	{
		coordSys = inCoordSys;
		initialise(coordSys);
	}
	
	public void finalize()
	{
		dispose();
	}

	/**
	 * Convert the given coordinate in display space to the local coordinate system.
	 * 
	 * @param disp Point in display coordinates.
	 * @return Point in the local coordinate system.
	 */
	public native Point3d displayToLocal(Point3d disp);
	
	/**
	 * Convert from the local coordinate system to display space.
	 * 
	 * @param local Point in the local system.
	 * @return Point in the display system.
	 */
	public native Point3d localToDisplay(Point3d local);
	
	/**
	 * Return the bounding box of the given display adapter.  This will typically be
	 * what the coordinate system can support.
	 * 
	 * @param ll Lower left corner of the bounding box.
	 * @param ur Upper right corner of bounding box.
	 */
	public native void getBounds(Point3d ll,Point3d ur);
	
	/**
	 * The coordinate system we're using for local coordinates.  Display
	 * coordinates are a scaled version of that in 2D and spherical for the globe.
	 * 
	 * @return
	 */
	public CoordSystem getCoordSystem()
	{
		return coordSys;
	}

	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise(CoordSystem coordSys);
	native void dispose();
	private long nativeHandle;
}
