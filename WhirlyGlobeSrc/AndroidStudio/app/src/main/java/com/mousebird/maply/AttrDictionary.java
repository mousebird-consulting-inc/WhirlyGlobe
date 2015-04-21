/*
 *  AttrDictionary.java
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
 * The attribute dictionary is a collection of name/value pairs.
 * We use it to store (and modify) attributes on VectorObject structures,
 * primarily.  It's analogous to the NSDictionary class on iOS, but much
 * simpler.
 *
 */
public class AttrDictionary
{	
	/**
	 * Construct an empty attribution dictionary
	 */
	AttrDictionary()
	{
		initialise();
	}
	
	/**
	 * Return a string corresponding to the given attribute name.
	 * @param attrName Name of the attribute we're looking for.
	 * @return Returns a string for the attribute or null if there was none.
	 */
	public native String getString(String attrName);
	
	/**
	 * Return an Integer corresponding to the given attribute name.
	 * @param attrName Name of the attribute we're looking for.
	 * @return Returns an Integer for the given attribute or null if there was none.
	 */
	public native Integer getInt(String attrName);
	
	/**
	 * Fetch a double corresponding to the given attribute name.
	 * @param attrName Name of the attribute we're looking for.
	 * @return Returns a Double for the given attribute or null if there was none.
	 */
	public native Double getDouble(String attrName);	
	
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
