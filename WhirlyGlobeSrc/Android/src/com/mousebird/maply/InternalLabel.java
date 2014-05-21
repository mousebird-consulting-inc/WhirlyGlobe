package com.mousebird.maply;

/**
 * An internal representation for the labels.  Toolkit users use ScreenLabel instead of this.
 * 
 * @author sjg
 *
 */
class InternalLabel 
{
	InternalLabel()
	{
		initialise();
	}
	
	InternalLabel(ScreenLabel label,LabelInfo info)
	{
		initialise();
		setLoc(label.loc);
		setRotation(label.rotation);
		if (label.text != null)
			setText(label.text);
		if (label.offset != null)
			setOffset(label.offset);
		setSelectable(label.selectable);
	}
	
	public native void setLoc(Point2d loc);
	public native void setRotation(double rotation);
	public native void setText(String text);
//	public native void iconImage(Bitmap image);
//	public native void setIconSize(Point2d iconSize);
	public native void setOffset(Point2d offset);
	// Note: Color
	public native void setSelectable(boolean selectable);
	// Note: Layout placement
	
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise();
	native void dispose();
	private long nativeHandle;	
}
