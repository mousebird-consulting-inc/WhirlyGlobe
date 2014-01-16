package com.mousebirdconsulting.maply;

import android.graphics.Color;

class InternalMarker 
{
	InternalMarker()
	{
		initialise();
	}
	
	InternalMarker(ScreenMarker marker,MarkerInfo info)
	{
		initialise();
		
		setLoc(marker.loc);
		setColor(Color.red(marker.color)/255.f,Color.green(marker.color)/255.f,Color.blue(marker.color)/255.f,Color.alpha(marker.color)/255.f);
		setRotation(marker.rotation);
		setHeight(marker.size.getX());
		setWidth(marker.size.getY());
		setOffset(marker.offset);
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
	public native void setOffset(Point2d offset);
	public native void setLayoutImportance(double layoutImp);
	
	public native void initialise();
	public native void dispose();
	private long nativeHandle;
}
