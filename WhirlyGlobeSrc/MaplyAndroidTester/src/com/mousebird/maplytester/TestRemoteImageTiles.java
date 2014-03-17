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
 * @author sjg
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
