/*
 *  AngleAxis.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/20/15.
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
 * The AngleAxis object wraps the C++ Eigen AngleAxisd object.
 * 
 * <p>
 * You'll probably only encounter them if you subclass MaplyController and
 * take over the gesture handling.
 *
 */
public class AngleAxis 
{
	AngleAxis()
	{
		initialise();
	}

	/**
	 * Construct with an angle and a vector around which to rotate.
	 * 
	 * @param ang
	 * @param vec
	 */
	AngleAxis(double ang,Point3d vec)
	{
		initialise(ang,vec);
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
	native void initialise(double ang,Point3d vec);
	native void dispose();
	private long nativeHandle;
}
