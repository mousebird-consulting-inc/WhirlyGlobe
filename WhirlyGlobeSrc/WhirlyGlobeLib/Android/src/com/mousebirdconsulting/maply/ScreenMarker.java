package com.mousebirdconsulting.maply;

import android.graphics.Bitmap;

/** @brief The Screen Marker is a 2D object that displays an image on the screen tracking a given location.
	@details The screen marker will track the given geographic location and display a centered rectangle with the given image.  Essentially it's a free floating icon, similar to the MaplyScreenLabel and it will always be drawn on top of other objects.  If the location would be behind the globe (in globe mode), the marker will disappear.
	@details If you're looking for a 3D marker object, that's the Marker.
*/
public class ScreenMarker 
{
	// Coordinate in geographic
	public Point2d loc = null;
	
	// Size on screen (in points)
	public Point2d size = new Point2d(16,16);
	
	// Set if we're rotating the marker
	public double rotation = 0.0;

	// Image we're showing for the texture
	public Bitmap image = null;
	
	// Background color
	public int color = 0xFFFFFFFF;
	
	// Importance value for the layout engine
//	public float layoutImportance = Float.MAX_VALUE;
	
	// On screen offset
	public Point2d offset = null;
	
	// If set, this object is selectable
//	public boolean selectable = false;
	
	// A user accessible object we can track for selection
	public Object userObject = null;
}
