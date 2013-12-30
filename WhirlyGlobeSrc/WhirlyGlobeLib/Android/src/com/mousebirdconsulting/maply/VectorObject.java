/*
 *  VectorObject
 *  com.mousebirdconsulting.maply
 *
 *  Created by Steve Gifford on 12/30/13.
 *  Copyright 2013 mousebird consulting
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

package com.mousebirdconsulting.maply;

/** Maply Vector Object represents zero or more vector features.
 * <p>
 * The Vector Object can hold several vector features of the same or different types.  It's meant to be a fairly opaque structure, 
 * often read from GeoJSON or Shapefiles.  It's less opaque than originally planned, however, and sports a number of specific methods.
 * <p> 
 * If you're doing real vector manipulation, it's best to do it somewhere else and then create one of these as needed for display.
 * <p>
 * Vector Objects can be created directly or read from a MaplyVectorDatabase.  They are typically then displayed on top of a 
 * MaplyViewController or WhirlyGlobeViewController as vectors.
 * <p>
 * Vector Objects vertices are always in geographic, with longitude = x and latitude = y.
*/
public class VectorObject 
{
	/**
	 * Construct empty.
	 */
	public VectorObject()
	{
		initialise();
	}
	
	public void finalize()
	{
		dispose();
	}
	
	/**
	 * Load vector objects from a GeoJSON string.
	 * @param json The GeoSJON string, presumably read from a file or over the network
	 * @return false if we were unable to parse the GeoJSON
	 */
	public native boolean fromGeoJSON(String json);
	
	public native void initialise();
	
	/**
	 * This will explicitly release memory associated with the native vector data.
	 */
	public native void dispose();

	private long nativeHandle;	
}
