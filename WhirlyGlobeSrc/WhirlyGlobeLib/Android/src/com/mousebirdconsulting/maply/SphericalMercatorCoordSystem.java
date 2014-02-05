package com.mousebirdconsulting.maply;

/**
 * A wrapper around Maply's spherical mercator coordinate system implementation.
 * Spherical mercator is the mostly commonly used coordinate system in
 * tile based implementations on the web.
 * 
 * @author sjg
 *
 */
public class SphericalMercatorCoordSystem extends CoordSystem
{
	/**
	 * Construct a spherical mercator system that covers the full
	 * extents of the earth.
	 */
	SphericalMercatorCoordSystem()
	{
		// Initialize to cover the whole world
		initialise();
		ll = geographicToLocal(new Point3d(-Math.PI,-85.05113/180.0*Math.PI,0.0));
		ur = geographicToLocal(new Point3d(Math.PI,85.05113/180.0*Math.PI,0.0));
	}

	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise();
	private long nativeHandle;
}
