package com.mousebird.maply;

/**
 * Base class for identifiable objects on the Maply side.
 * These are basically just unique IDs.
 */
public class Identifiable 
{
	/**
	 * Generate a new ID.  This hooks into the Identifiables on the Maply C++ side.
	 */
	public static native long genID();
}
