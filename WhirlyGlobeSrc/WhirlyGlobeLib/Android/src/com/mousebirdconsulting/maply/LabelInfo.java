package com.mousebirdconsulting.maply;

/**
 * We use this class to designate attribute shared by a group of labels.
 * Rather than specifying them individually they all go here.  You would
 * use this class in the course of an addLabels() or addScreenLabels()
 * call on the MaplyController.
 * 
 * @author sjg
 *
 */
public class LabelInfo 
{
	/**
	 * Controls whether or not the geometry will be visible.  By
	 * default this is true.
	 * @param newEnable New value for the enable.
	 */
	void setEnable(boolean newEnable)
	{
		enable = newEnable;
	}
	boolean enable = true;

	/**
	 * Controls the font size used for the text.
	 * 
	 * @param newSize The new size value
	 */
	void setFontSize(float newSize)
	{
		fontSize = newSize;
	}
	float fontSize = 16.f;

	/**
	 * The amount of time (in seconds) it takes for new geometry
	 * to fade in and fade out.  By default, fade is off.
	 * 
	 * @param newFade The new fade value.
	 */
	void setFade(float newFade)
	{
		fade = newFade;
	}
	float fade = 0.0f;
}
