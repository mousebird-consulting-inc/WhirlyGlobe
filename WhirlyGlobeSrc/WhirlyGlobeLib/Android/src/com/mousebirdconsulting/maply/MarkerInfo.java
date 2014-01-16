package com.mousebirdconsulting.maply;

public class MarkerInfo 
{
	MarkerInfo()
	{
		initialise();
	}
	
	public void finalize()
	{
		dispose();
	}
	
	public native void setEnable(boolean enable);
	public native void setDrawOffset(float drawOffset);
	public native void setDrawPriority(int drawPriority);
	public native void setMinVis(float minVis);
	public native void setMaxVis(float maxVis);
	public native void setColor(float r,float g,float b,float a);
	public native void setFade(float fade);

	public native void initialise();
	public native void dispose();
	private long nativeHandle;
}
