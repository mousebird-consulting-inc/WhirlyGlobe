/*
 *  VectorInfo.java
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
 * The Vector Info class holds visual information related to groups of vectors.
 * For efficiency's sake we put visual info in this class, rather than on
 * attributes in the vectors themselves.  There may be attributes that can
 * override these, however.
 *
 */
public class VectorInfo extends BaseInfo
{
	/**
	 * Construct the vector info empty with default values.  At the very least
	 * a vector will be white and visible all the time.
	 */
	public VectorInfo()
	{
		initialise();
	}
	
	public void finalize()
	{
		dispose();
	}

	/**
	 * Set whether or not areal features are tesselated and draw as filled.
	 * Default is fault.
	 */
	public native void setFilled(boolean filled);
	
//	public native void setTexId(long texId);
//	public native void setTexScale(float s,float t);
//	public native void subdivEps(float eps);
//	public native void setGridSubdiv(boolean gridSubdiv);
	
	/**
	 * Set the color used by the geometry.  Color values range from 0 to 1.0.
	 * You must specify all four values.  Alpha controls transparency.
	 * @param r Red component.
	 * @param g Green component.
	 * @param b Blue component.
	 * @param a Alpha component.
	 */
	public native void setColor(float r,float g,float b,float a);

	/**
	 * This is the line width for vector features.  By default this is 1.0.
	 */
	public native void setLineWidth(float lineWidth);
	
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise();
	native void dispose();
}
