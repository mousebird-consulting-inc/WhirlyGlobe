/*
 *  VectorIterator.java
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

import java.util.Iterator;

/**
 * A VectorObject can contain multiple sub-vectors.  This iterator
 * lets toolkit users iterate over those sub-vectors without having
 * to access them directly.
 *
 */
public class VectorIterator implements Iterator<VectorObject>
{
	private VectorIterator()
	{
	}
	
	/**
	 * Construct with a vector object.
	 */
	VectorIterator(VectorObject vecObj)
	{
		initialise(vecObj);
	}

	static
	{
		nativeInit();
	}
	public void finalize() {
		dispose();
	}
	private static native void nativeInit();
	native void initialise(VectorObject vecObj);
	native void dispose();

	private long nativeHandle;

	/**
	 * Returns true if there's another vector object in the list.
	 */
	@Override
	public native boolean hasNext();

	/**
	 * Return the next vector object in the list.
	 */
	@Override
	public native VectorObject next();

	/**
	 * This will do nothing.
	 */
	@Override
	public native void remove();
}
