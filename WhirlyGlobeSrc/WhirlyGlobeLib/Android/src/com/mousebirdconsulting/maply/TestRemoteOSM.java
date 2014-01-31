package com.mousebirdconsulting.maply;

import java.io.File;

import android.app.Activity;

public class TestRemoteOSM 
{
	Activity activity = null;
	MaplyController mapControl = null;
	
	TestRemoteOSM(Activity inActivity,MaplyController inMapControl)
	{
		activity = inActivity;
		mapControl = inMapControl;
	}
	
	void start()
	{
		// Cache directory for tiles
		File cacheDir = new File(activity.getCacheDir(),"osmtiles");
		cacheDir.mkdir();

		// Set up a paging layer in spherical mercator and the proper delegate
		SphericalMercatorCoordSystem coordSys = new SphericalMercatorCoordSystem();
		OSMVectorTilePager pager = new OSMVectorTilePager(mapControl,"http://tile.openstreetmap.us/vectiles-all/",0,16);
		pager.cacheDir = cacheDir;
//		TestQuadPager pager = new TestQuadPager(0,16);
		QuadPagingLayer pagingLayer = new QuadPagingLayer(mapControl,coordSys, pager);
		mapControl.layerThread.addLayer(pagingLayer);
	}
}
