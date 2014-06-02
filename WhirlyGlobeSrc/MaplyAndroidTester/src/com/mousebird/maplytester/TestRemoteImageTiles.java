/*
 *  TestRemoteImageTiles.java
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
import com.mousebird.maply.MaplyTileID;
import com.mousebird.maply.QuadImageTileLayer;
import com.mousebird.maply.RemoteTileSource;
import com.mousebird.maply.SphericalMercatorCoordSystem;

import android.app.Activity;
import android.util.Log;

/**
 * Test Maply by creating a quad image tile layer with a remote
 * tile source.
 *
 */
public class TestRemoteImageTiles implements RemoteTileSource.TileSourceDelegate 
{
	Activity activity = null;
	MaplyController mapControl = null;
	
	TestRemoteImageTiles(Activity inActivity,MaplyController inMapControl)
	{
		activity = inActivity;
		mapControl = inMapControl;
	}
	
	public void start()
	{
		// Cache directory for tiles
		File cacheDir = new File(activity.getCacheDir(),"testbasemap");
		cacheDir.mkdir();
		
		SphericalMercatorCoordSystem coordSys = new SphericalMercatorCoordSystem();
		RemoteTileSource tileSource = new RemoteTileSource("http://a.tiles.mapbox.com/v3/examples.map-zyt2v9k2/","png",0,22);
		tileSource.setCacheDir(cacheDir);
		tileSource.delegate = this;
		QuadImageTileLayer layer = new QuadImageTileLayer(mapControl,coordSys,tileSource);
		layer.setSimultaneousFetches(8);
		mapControl.getLayerThread().addLayer(layer);
	}

	@Override
	public void tileDidLoad(RemoteTileSource tileSource, MaplyTileID tileID) 
	{
//		Log.i("Maply","Loaded tile " + tileID.level + ": (" + tileID.x + "," + tileID.y + ")");
	}

	@Override
	public void tileDidNotLoad(RemoteTileSource tileSource, MaplyTileID tileID) 
	{
		Log.i("Maply","Failed to load tile " + tileID.level + ": (" + tileID.x + "," + tileID.y + ")");
	}
}
