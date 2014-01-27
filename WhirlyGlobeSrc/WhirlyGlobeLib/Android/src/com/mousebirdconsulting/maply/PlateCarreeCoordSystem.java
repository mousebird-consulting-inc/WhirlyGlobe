package com.mousebirdconsulting.maply;

public class PlateCarreeCoordSystem extends CoordSystem
{
	PlateCarreeCoordSystem()
	{
		initialise();
		// Initialize to cover the whole world
		ll = geographicToLocal(new Point3d(-Math.PI,-Math.PI/2.0,0.0));
		ur = geographicToLocal(new Point3d(Math.PI,Math.PI/2.0,0.0));
	}

	public native void initialise();
	private long nativeHandle;
}
