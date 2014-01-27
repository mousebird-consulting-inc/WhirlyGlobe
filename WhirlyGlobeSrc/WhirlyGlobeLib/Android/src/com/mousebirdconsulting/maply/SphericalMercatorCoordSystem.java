package com.mousebirdconsulting.maply;

public class SphericalMercatorCoordSystem extends CoordSystem
{
	SphericalMercatorCoordSystem()
	{
		// Initialize to cover the whole world
		initialise();
		ll = geographicToLocal(new Point3d(-Math.PI,-85.05113/180.0*Math.PI,0.0));
		ur = geographicToLocal(new Point3d(Math.PI,85.05113/180.0*Math.PI,0.0));
	}

	public native void initialise();
	private long nativeHandle;
}
