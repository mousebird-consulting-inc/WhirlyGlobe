package com.mousebirdconsulting.maply;

/**
 * The screen label is a 2D label that tracks a location on the map.
 * 
 * @author sjg
 *
 */
public class ScreenLabel 
{
	// Coordinate in geographic
	public Point2d loc = null;
	
	// Set if we're rotating the marker
	public double rotation = 0.0;

	// Size on screen (in points)
	public Point2d size = new Point2d(16,16);
	
	// Text to display
	public String text;
		
	// Importance value for the layout engine
//	public float layoutImportance = Float.MAX_VALUE;
	
	// On screen offset
	public Point2d offset = null;
	
	// If set, this object is selectable
//	public boolean selectable = false;
	
	// A user accessible object we can track for selection
	public Object userObject = null;
}
