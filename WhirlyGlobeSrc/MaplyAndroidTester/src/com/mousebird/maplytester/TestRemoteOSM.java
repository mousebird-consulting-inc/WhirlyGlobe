package com.mousebird.maplytester;

import java.io.File;

import com.mousebird.maply.MaplyController;
import com.mousebird.maply.QuadPagingLayer;
import com.mousebird.maply.SphericalMercatorCoordSystem;
import com.mousebird.maply.OSMVectorTilePager;

import android.app.Activity;

/**
 * Test Maply by creating a quad paging layer with an OSMVectorTilePager.
 * This is effectively an OSM vector map.
 * 
 * @author sjg
 *
 */
public class TestRemoteOSM 
{
	Activity activity = null;
	MaplyController mapControl = null;
	
	/**
	 * Construct with the main activity and maply controller.  Won't actually
	 * do anything, though, call start() for that.
	 */
	TestRemoteOSM(Activity inActivity,MaplyController inMapControl)
	{
		activity = inActivity;
		mapControl = inMapControl;
	}
	
	/**
	 * This kicks off the action, creating the quad paging layer and paging
	 * delegate.
	 */
	public void start()
	{
		// Cache directory for tiles
		File cacheDir = new File(activity.getCacheDir(),"osmtiles");
		cacheDir.mkdir();

		// Set up a paging layer in spherical mercator and the proper delegate
		SphericalMercatorCoordSystem coordSys = new SphericalMercatorCoordSystem();
		int numThreads = 8;
		OSMVectorTilePager pager = new OSMVectorTilePager(mapControl,"http://tile.openstreetmap.us/vectiles-all/",0,16,numThreads);
		pager.setCacheDir(cacheDir);
//		TestQuadPager pager = new TestQuadPager(0,16);
		QuadPagingLayer pagingLayer = new QuadPagingLayer(mapControl,coordSys, pager);
		pagingLayer.setSimultaneousFetches(numThreads);
		pagingLayer.setSingleLevelLoading(true);
		pagingLayer.setUseTargetZoomLevel(true);
		pagingLayer.setImportance(256*256);
		mapControl.getLayerThread().addLayer(pagingLayer);
//		mapControl.renderWrapper.maplyRender.setClearColor(1.f,1.f,1.f,1.f);
	}
}
