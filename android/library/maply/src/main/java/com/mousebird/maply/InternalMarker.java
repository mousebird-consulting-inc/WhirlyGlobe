/*
 *  InternalMarker.java
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
 * An internal representation for the markers.  Toolkit users use ScreenMarker or Marker instead of this.
 *
 */
class InternalMarker
{
	InternalMarker()
	{
		initialise();
	}

	// Basic setup for both types of screen markers
	private void screenMakerSetup(ScreenMarker marker)
	{
		if (marker.selectable)
			setSelectID(marker.ident);
		setLoc(marker.loc);
		setColor(Color.red(marker.color)/255.f,Color.green(marker.color)/255.f,Color.blue(marker.color)/255.f,Color.alpha(marker.color)/255.f);
		if (marker.rotation != 0.0)
			setRotation(marker.rotation);
		// Note: Lock rotation?
		setSize(marker.size.getX(),marker.size.getY());
		if (marker.layoutSize != null)
			setLayoutSize(marker.layoutSize.getX(),marker.layoutSize.getY());
		if (marker.offset != null)
			setOffset(marker.offset.getX(),marker.offset.getY());
		setPeriod(marker.period);
		if (marker.vertexAttributes != null)
			setVertexAttributes(marker.vertexAttributes.toArray());
	}
	
	/**
	 * Construct with the screen marker we want to represent and how it looks.
	 * 
	 * @param marker Screen marker to represent.
	 * @param info How the screen marker should look.
	 */
	InternalMarker(ScreenMarker marker)
	{
		initialise();

		screenMakerSetup(marker);
	}

	/**
	 * 2D moving marker
	 */
	InternalMarker(ScreenMovingMarker marker,double startTime)
	{
		initialise();

		screenMakerSetup(marker);

		setEndLoc(marker.endLoc);
		setAnimationRange(startTime,startTime+marker.duration);
	}

	/**
	 * Construct with a 3D marker.
     */
	InternalMarker(Marker marker)
	{
		initialise();

		if (marker.selectable)
			setSelectID(marker.ident);
		setLoc(marker.loc);
		setColor(Color.red(marker.color)/255.f,Color.green(marker.color)/255.f,Color.blue(marker.color)/255.f,Color.alpha(marker.color)/255.f);
		setSize(marker.size.getX(),marker.size.getY());
		setPeriod(marker.period);
	}

	public void finalize()
	{
		dispose();
	}

	public native void setSelectID(long selectID);
	public native void setLoc(Point2d loc);
	public native void setEndLoc(Point2d loc);
	public native void setAnimationRange(double startTime,double endTime);
	public native void setColor(float r,float g,float b,float a);
	public native void addTexID(long texID);
	public native void setRotation(double rot);
	public native void setLockRotation(boolean lockRotation);
	public native void setSize(double width,double height);
	public native void setLayoutSize(double width,double height);
	public native void setOffset(double offX,double offY);
	public native void setPeriod(double period);
	public native void setVertexAttributes(Object vertAttrs[]);

	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise();
	native void dispose();
	private long nativeHandle;
}
