package com.mousebirdconsulting.maply;

class VectorInfo 
{
	VectorInfo()
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
	public native void setFilled(boolean filled);
	public native void setTexId(long texId);
	public native void setTexScale(float s,float t);
	public native void subdivEps(float eps);
	public native void setGridSubdiv(boolean gridSubdiv);
	public native void setColor(float r,float g,float b,float a);
	public native void setFade(float fade);
	public native void setLineWidth(float lineWidth);
	
	public native void initialise();
	public native void dispose();
	private long nativeHandle;
}
