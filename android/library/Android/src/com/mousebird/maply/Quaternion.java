/*
 *  Quaternion.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/18/15.
 *  Copyright 2011-2015 mousebird consulting
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
 * This encapsulates a Quaternion, used in manipulation. This is largely
 * opaque and is just passed around as needed.
 * <p>
 * You'll probably only encounter them if you subclass MaplyController and
 * take over the gesture handling.
 *
 */
public class Quaternion 
{
	Quaternion()
	{
		initialise();
	}

	/**
	 * Construct with the difference between two vectors.
	 * 
	 * @param vec1
	 * @param vec2
	 */
	Quaternion(Point3d vec1,Point3d vec2)
	{
		initialise(vec1,vec2);
	}
	
	public void finalize()
	{
		dispose();
	}
	
	// Multiply this quaternion with the other and return it
	public native Quaternion multiply(Quaternion other);
	
	// Multiply a vector through the quaternion to produce a vector
	public native Point3d multiply(Point3d vec);
	
	// Multiply the Quaternion through the angle axis to produce a new quaternion
	public native Quaternion multiply(AngleAxis angAxis);
	
	// Linear interpolation between this Quaternion and that one
	public native Quaternion slerp(Quaternion that,double t);
		
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise();
	native void initialise(Point3d vec1,Point3d vec2);
	native void dispose();
	private long nativeHandle;
}
