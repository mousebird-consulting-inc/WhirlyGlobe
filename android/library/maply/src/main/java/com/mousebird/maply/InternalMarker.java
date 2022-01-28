/*  InternalMarker.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2022 mousebird consulting
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
 */
package com.mousebird.maply;

import android.graphics.Color;

import androidx.annotation.ColorInt;

/**
 * An internal representation for the markers.  Toolkit users use ScreenMarker or Marker instead of this.
 */
@SuppressWarnings("unused")
class InternalMarker
{
	protected InternalMarker()
	{
		initialise();
	}

	// Basic setup for both types of screen markers
	private void screenMarkerSetup(ScreenMarker marker)
	{
		setLoc(marker.loc);
		setColor(marker.color);
		setPeriod(marker.period);
		setLayoutImportance(marker.layoutImportance);

		// Note: Lock rotation?

		if (marker.selectable) {
			setSelectID(marker.ident);
		}
		if (marker.rotation != 0.0) {
			setRotation(marker.rotation);
		}
		if (marker.size != null) {
			setSizePt(marker.size);
		}
		if (marker.layoutSize != null) {
			setLayoutSizePt(marker.layoutSize);
		}
		if (marker.offset != null) {
			setOffsetPt(marker.offset);
		}
		if (marker.vertexAttributes != null) {
			setVertexAttributes(marker.vertexAttributes.toArray());
		}
		if (marker.orderBy != 0) {
			setOrderBy(marker.orderBy);
		}
		if (marker.uniqueID != null && !marker.uniqueID.isEmpty()) {
			setUniqueID(marker.uniqueID);
		}
	}
	
	/**
	 * Construct with the screen marker we want to represent and how it looks.
	 * 
	 * @param marker Screen marker to represent.
	 */
	InternalMarker(ScreenMarker marker)
	{
		initialise();

		screenMarkerSetup(marker);
	}

	/**
	 * 2D moving marker
	 */
	InternalMarker(ScreenMovingMarker marker,double startTime)
	{
		initialise();

		screenMarkerSetup(marker);

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
		setColor(marker.color);
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
	public native void setColor(@ColorInt int color);
	public native void setColorComponents(float r,float g,float b,float a);
	public native void addTexID(long texID);
	public native void setRotation(double rot);
	public native void setLockRotation(boolean lockRotation);
	public native void setSize(double width,double height);
	public native void setSizePt(Point2d size);
	public native void setLayoutSize(double width,double height);
	public native void setLayoutSizePt(Point2d size);
	public native void setOffset(double offX,double offY);
	public native void setOffsetPt(Point2d offset);
	public native void setPeriod(double period);
	public native void setVertexAttributes(Object[] vertexAttributes);
	public native void setLayoutImportance(float importance);
	public native void setOrderBy(int order);
	public native void setUniqueID(String id);

	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise();
	native void dispose();
	private long nativeHandle;
}
