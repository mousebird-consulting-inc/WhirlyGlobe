package com.mousebirdconsulting.maply;

/**
 * The screen label is a 2D label that tracks a location on the map.
 * The label has a geographic location as well as information about
 * the text it's displaying and other control attributes.
 * <p>
 * It works in combination with a LabelInfo class to describe more
 * broad attributes such as color and font size.
 * 
 * @author sjg
 *
 */
public class ScreenLabel 
{
	/**
	 * The location in geographic (WGS84) radians.  x is longitude, y is latitude.
	 * The label will track this location.
	 */
	public Point2d loc = null;

	/**
	 * If non-zero we'll rotate the label that number of radians around the attachment
	 * point.
	 */
	public double rotation = 0.0;

	/**
	 * Size of the label on screen.
	 */
	public Point2d size = new Point2d(16,16);

	/**
	 * The text to display for the label.
	 */
	public String text;
		
	// Importance value for the layout engine
//	public float layoutImportance = Float.MAX_VALUE;

	/**
	 * If non-null an offset to tweak the label by.  We'll move the label
	 * this number of pixels from where it normally would be.
	 */
	public Point2d offset = null;
	
	// If set, this object is selectable
//	public boolean selectable = false;

	/**
	 * For selection, we include a user accessible object pointer.  You use
	 * this to identify a label if you're doing user selection.
	 */
	public Object userObject = null;
}
