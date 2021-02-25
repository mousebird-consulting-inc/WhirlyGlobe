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

import android.graphics.Color;

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
	 * Default draw priority for vector features.
	 */
	public static int VectorPriorityDefault = 50000;

	/**
	 * Construct the vector info empty with default values.  At the very least
	 * a vector will be white and visible all the time.
	 */
	public VectorInfo()
	{
		initialise();
		setFilled(false);
		setColor(1.f,1.f,1.f,1.f);
		setLineWidth(1.f);
		setDrawPriority(VectorPriorityDefault);
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

	/**
	 * Vectors can be subdivided to follow a globe.
	 * If set, this is the offset from the globe that will trigger
	 * a subdivision.
	 */
	public native void setSampleEpsilon(double eps);

	/**
	 * If set and filled is set, we will apply the given texture across any areal features.
	 * How the texture is applied can be controlled by the textScale, textureProjection, and vecCenter.
	 */
	public void setTexture(MaplyTexture tex)
	{
		setTextureID(tex.texID);
	}

	/**
	 * Set the texture to use on a vector by ID.
	 */
	native void setTextureID(long texID);

	/**
	 * These control the scale of the texture application.  We'll multiply by these numbers before
	 * generating texture coordinates from the vertices.
	 */
	public native void setTexScale(double u,double v);

	/**
	 * If set, we'll subdivide areal features on top of the globe
	 * using a grid to figure out the optimal subdivision.
	 * This is the vertical distance from the globe to use as a
	 * trigger for more subdivision.
	 */
	public native void setSubdivEps(double eps);

	/**
	 * When using textures on areal features, you can project the texture a couple of different ways.
	 * Using TangentPlane works well for the globe and Screen works well in 2D.
	 */
	public enum TextureProjection {None,TangentPlane,Screen};

	public void setTextureProjection(TextureProjection texProjection)
	{
		setTextureProjectionNative(texProjection.ordinal());
	}

	native void setTextureProjectionNative(int texProjection);

	/**
	 * Set the color used by the geometry.
	 * @param color Color in Android format, including alpha.
     */
	public void setColor(int color)
	{
		setColor(Color.red(color)/255.f,Color.green(color)/255.f,Color.blue(color)/255.f,Color.alpha(color)/255.f);
	}

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

	/**
	 * If set, we'll calculate a center for the vector geometry and
	 * use that in building the visual geometry.  This is set on
	 * by default when we pass in a center as well.
	 */
	native public void setUseCenter();

	/**
	 * These control the center of a texture application.  If not set we'll use the areal's centroid.
	 * If set, we'll use these instead.  They should be in local coordinates (probably geographic radians).
	 */
	public void setVecCenter(Point2d center)
	{
		setVecCenterNative(center.getX(),center.getY());
	}

	native void setVecCenterNative(double x,double y);

	// Convert to a string for debugging
	public native String toString();

	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise();
	native void dispose();
}
