package com.mousebirdconsulting.maply;

/**
 * The Plate Carree coordinate system is just a fancy name for
 * stretching lat/lon out onto a plane in the dumbest way possible.
 * <p>
 * This coordinate system is often used by display adapters or
 * the contents of paging layers.
 * 
 * @author sjg
 *
 */
public class PlateCarreeCoordSystem extends CoordSystem
{
	/**
	 * Construct the coordinate system to cover the whole world.
	 */
	PlateCarreeCoordSystem()
	{
		initialise();
		// Initialize to cover the whole world
		ll = geographicToLocal(new Point3d(-Math.PI,-Math.PI/2.0,0.0));
		ur = geographicToLocal(new Point3d(Math.PI,Math.PI/2.0,0.0));
	}

	native void initialise();
	private long nativeHandle;
}
