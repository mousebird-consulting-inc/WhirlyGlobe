package com.mousebird.maplytester;

import java.io.File;

import android.app.Activity;

import com.mousebird.maply.MaplyController;
import com.mousebird.maply.QuadImageTileLayer;
import com.mousebird.maply.SphericalMercatorCoordSystem;
import com.mousebird.maply.TestImageSource;

public class TestImagePagerSource 
{
	Activity activity = null;
	MaplyController mapControl = null;
	
	TestImagePagerSource(Activity inActivity,MaplyController inMapControl)
	{
		activity = inActivity;
		mapControl = inMapControl;
	}
	
	public void start()
	{
		SphericalMercatorCoordSystem coordSys = new SphericalMercatorCoordSystem();
		TestImageSource tileSource = new TestImageSource(activity.getMainLooper(),0,22);
		QuadImageTileLayer layer = new QuadImageTileLayer(mapControl,coordSys,tileSource);
		layer.setSimultaneousFetches(4);
		mapControl.getLayerThread().addLayer(layer);
	}
}
