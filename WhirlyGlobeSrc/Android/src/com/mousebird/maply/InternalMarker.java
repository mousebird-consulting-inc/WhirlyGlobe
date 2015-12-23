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
	
	/**
	 * Construct with the screen marker we want to represent and how it looks.
	 * 
	 * @param marker Screen marker to represent.
	 * @param info How the screen marker should look.
	 */
	InternalMarker(ScreenMarker marker,MarkerInfo info)
	{
		initialise();
		
		setLoc(marker.loc);
		setColor(Color.red(marker.color)/255.f,Color.green(marker.color)/255.f,Color.blue(marker.color)/255.f,Color.alpha(marker.color)/255.f);
		setRotation(marker.rotation);
		setWidth(marker.size.getX());
		setHeight(marker.size.getY());
		setLayoutImportance(marker.layoutImportance);
		setSelectable(marker.selectable);
		if (marker.offset == null)
			setOffset(0,0);
		else
			setOffset(marker.offset.getX(),marker.offset.getY());
		if (marker.selectable)
			setSelectID(marker.ident);
	}
	
	public void finalize()
	{
		dispose();
	}

	public native void setSelectable(boolean sel);
	public native void setSelectID(long selectID);
	public native void setLoc(Point2d loc);
	public native void setColor(float r,float g,float b,float a);
	public native void addTexID(long texID);
	public native void setLockRotation(boolean lockRotation);
	public native void setHeight(double height);
	public native void setWidth(double width);
	public native void setRotation(double rot);
	public native void setOffset(double offX,double offY);
	public native void setLayoutImportance(double layoutImp);
	
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise();
	native void dispose();
	private long nativeHandle;
}
