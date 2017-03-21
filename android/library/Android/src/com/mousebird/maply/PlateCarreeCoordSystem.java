/*
 *  PlateCarreeCoordSystem.java
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
 * The Plate Carree coordinate system is just a fancy name for
 * stretching lat/lon out onto a plane in the dumbest way possible.
 * <p>
 * This coordinate system is often used by display adapters or
 * the contents of paging layers.
 * 
 * @author sjg
 *
 */
public class PlateCarreeCoordSystem extends CoordSystem
{
	/**
	 * Construct the coordinate system to cover the whole world.
	 */
	public PlateCarreeCoordSystem()
	{
		initialise();
		// Initialize to cover the whole world
		ll = geographicToLocal(new Point3d(-Math.PI,-Math.PI/2.0,0.0));
		ur = geographicToLocal(new Point3d(Math.PI,Math.PI/2.0,0.0));
	}

	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise();
}
