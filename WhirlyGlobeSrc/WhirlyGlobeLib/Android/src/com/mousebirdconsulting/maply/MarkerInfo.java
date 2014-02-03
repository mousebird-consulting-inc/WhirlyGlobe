package com.mousebirdconsulting.maply;

/**
 * This class holds the visual information for a set of 2D or 3D markers.
 * Rather than have each of those represent their own visual information,
 * we share it here.
 * <p>
 * Toolkit users fill this class out and pass it into the addScreenMarkers()
 * or addMarkers() call on the MaplyController.
 * 
 * @author sjg
 *
 */
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

	native void initialise();
	native void dispose();
	private long nativeHandle;
}
