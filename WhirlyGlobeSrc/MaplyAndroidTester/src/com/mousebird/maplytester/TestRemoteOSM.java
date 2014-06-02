/*
 *  TestRemoteOSM.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2014 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

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
		pagingLayer.setImportance(512*512);
		mapControl.getLayerThread().addLayer(pagingLayer);
//		mapControl.renderWrapper.maplyRender.setClearColor(1.f,1.f,1.f,1.f);
	}
}
