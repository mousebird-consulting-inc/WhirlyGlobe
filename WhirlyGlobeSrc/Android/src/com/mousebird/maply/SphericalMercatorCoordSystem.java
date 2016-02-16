/*
 *  SphericalMercatorCoordSystem.java
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
 * A wrapper around Maply's spherical mercator coordinate system implementation.
 * Spherical mercator is the mostly commonly used coordinate system in
 * tile based implementations on the web.
 *
 */
public class SphericalMercatorCoordSystem extends CoordSystem
{
	/**
	 * Construct a spherical mercator system that covers the full
	 * extents of the earth.
	 */
	public SphericalMercatorCoordSystem()
	{
		// Initialize to cover the whole world
		initialise();
		ll = geographicToLocal(new Point3d(-Math.PI,-85.05113/180.0*Math.PI,0.0));
		ur = geographicToLocal(new Point3d(Math.PI,85.05113/180.0*Math.PI,0.0));
	}

	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise();
}
