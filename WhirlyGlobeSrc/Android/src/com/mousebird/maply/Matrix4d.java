/*
 *  Matrix4d.java
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
 * This encapsulates a Maply4d matrix.  In general, we don't manipulate
 * them, but we do pass them around.
 * <p>
 * You'll probably only encounter them if you subclass MaplyController and
 * take over the gesture handling.
 *
 */
public class Matrix4d 
{
	Matrix4d()
	{
		initialise();
	}
	
	public void finalize()
	{
		dispose();
	}
		
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise();
	native void dispose();
	/**
	 * Return the inverse of this matrix.
	 */
	public native Matrix4d inverse();

	/**
	 * Transpose and return the matrix.
     */
	public native Matrix4d transpose();
	/**
	 * Multiply the vector by this matrix and return the result.
	 */
	public native Point4d multiply(Point4d vec);
	private long nativeHandle;
}
