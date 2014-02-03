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

import java.util.Map;

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
public class VectorObject implements Iterable<VectorObject>
{
	/**
	 * Construct empty.
	 */
	public VectorObject()
	{
		initialise();
	}
	
	// Return attributes for the feature.  If there are multiple features, we get the first one.
	public native AttrDictionary getAttributes();
	
	// Add a single point
	public native void addPoint(Point2d pt);

	// Add a linear feature
	public native void addLinear(Point2d pts[]);
	
	// Add an areal feature
	public native void addAreal(Point2d pts[]);
//	public native void addAreal(Point2d ext[],Point2d holes[][]);
	
	public void finalize()
	{
		dispose();
	}
	
	// Vector object can contain multiple subobjects.  This iterates over them.
	@Override
	public VectorIterator iterator() 
	{
		return new VectorIterator(this);
	}	
	
	/**
	 * Load vector objects from a GeoJSON string.
	 * @param json The GeoSJON string, presumably read from a file or over the network
	 * @return false if we were unable to parse the GeoJSON
	 */
	public native boolean fromGeoJSON(String json);
	
	/**
	 * Load vector objects from a GeoJSON assembly, which is just a bunch of GeoJSON stuck together.
	 * @param json
	 * @return
	 */
	static public native Map<String,VectorObject> FromGeoJSONAssembly(String json);
	
	public native boolean readFromFile(String fileName);
	public native boolean writeToFile(String fileName);
		
	public native void initialise();
	public native void dispose();

	private long nativeHandle;
}
