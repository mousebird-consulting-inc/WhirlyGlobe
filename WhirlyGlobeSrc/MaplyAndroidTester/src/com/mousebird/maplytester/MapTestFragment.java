package com.mousebird.maplytester;

import java.io.File;

import com.mousebird.maply.MapFragment;
import com.mousebird.maply.MaplyController;
import com.mousebird.maply.QuadImageTileLayer;
import com.mousebird.maply.QuadPagingLayer;
import com.mousebird.maply.RemoteTileSource;
import com.mousebird.maply.SphericalMercatorCoordSystem;
import com.mousebird.maply.TestImageSource;
import com.mousebird.maply.TestQuadPager;
import com.mousebird.maplytester.ConfigView.ConfigViewListener;

import android.app.Fragment;
import android.os.Bundle;
import android.support.v4.widget.DrawerLayout;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;

/**
 * This fragment manages the map (globe), adding and removing content as the user requests.
 * 
 */
public class MapTestFragment extends Fragment implements ConfigViewListener
{
	MaplyController mapControl = null;
	
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState)
	{
		DrawerLayout drawer = new android.support.v4.widget.DrawerLayout(getActivity());
	    DrawerLayout.LayoutParams lp = new DrawerLayout.LayoutParams(
	    		DrawerLayout.LayoutParams.MATCH_PARENT , DrawerLayout.LayoutParams.MATCH_PARENT);
		drawer.setLayoutParams(lp);

    	// Create the Maply Controller
    	mapControl = new MaplyController(getActivity());
    	View mapView = mapControl.getContentView();
	    DrawerLayout.LayoutParams mlp = new DrawerLayout.LayoutParams(
	    		DrawerLayout.LayoutParams.MATCH_PARENT , DrawerLayout.LayoutParams.MATCH_PARENT);
	    mapView.setLayoutParams(mlp);
	    drawer.addView(mapView);
    	
    	// And the overlaid config view
    	ConfigView configView = new ConfigView(getActivity());		
	    DrawerLayout.LayoutParams clp = new DrawerLayout.LayoutParams(
	    		DrawerLayout.LayoutParams.MATCH_PARENT , DrawerLayout.LayoutParams.MATCH_PARENT);
	    clp.gravity=Gravity.RIGHT;
    	configView.setLayoutParams(clp);
    	configView.setConfigViewListener(this);
		userChangedSelections(configView);
		drawer.addView(configView);
		
		return drawer;
	}
	
	@Override
	public void onDestroy()
	{
		super.onDestroy();
	}
	
	QuadImageTileLayer baseLayer = null;

	// Called when the user changes what is selected
	@Override
	public void userChangedSelections(ConfigView config) 
	{
		String cacheDirName = null;
		QuadImageTileLayer.TileSource tileSource = null;
		RemoteTileSource remoteTileSource = null;
		
		// Get rid of the existing base layer
		if (baseLayer != null)
		{
			mapControl.removeLayer(baseLayer);
		}
		
		for (ConfigView.ConfigEntry entry : config.sections.get(0).entries)
		{
			if (entry.status)
			{
				switch (entry.optionIdent)
				{
				case BlankLayer:
					break;
				case GeographyClass:
					// Note: Put MBTiles in resources
					break;
				case MapboxRegular:
//					cacheDirName = "mapbox_regular";
//					tileSource = new RemoteTileSource("http://a.tiles.mapbox.com/v3/examples.map-zswgei2n/","png",0,22);				
					break;
				case MapboxSatellite:
					cacheDirName = "mapbox_satellite";
					remoteTileSource = new RemoteTileSource("http://a.tiles.mapbox.com/v3/examples.map-zyt2v9k2/","png",0,22);
					break;
				case MapboxTerrain:
//					cacheDirName = "mapbox_terrain";
//					tileSource = new RemoteTileSource("http://a.tiles.mapbox.com/v3/examples.map-zq0f1vuc/","png",0,22);
					break;
				case OSMMapquest:
					cacheDirName = "osm_mapquest";
					remoteTileSource = new RemoteTileSource("http://otile1.mqcdn.com/tiles/1.0.0/osm/","png",0,18);
					break;
				case StamenWatercolor:
					cacheDirName = "stamen_watercolor";
					remoteTileSource = new RemoteTileSource("http://tile.stamen.com/watercolor/","png",0,18);
					break;
				case QuadTest:
					tileSource = new TestImageSource(getActivity().getMainLooper(),0,22);
					break;
				case QuadTestAnimate:
					break;
				case QuadVectorTest:
				{
					TestQuadPager testPager = new TestQuadPager(0,22);
					SphericalMercatorCoordSystem coordSys = new SphericalMercatorCoordSystem();
					QuadPagingLayer pagingLayer = new QuadPagingLayer(mapControl,coordSys,testPager);
					pagingLayer.setSimultaneousFetches(6);
					pagingLayer.setImportance(256*256);
					pagingLayer.setSingleLevelLoading(true);
					pagingLayer.setUseTargetZoomLevel(true);
					mapControl.addLayer(pagingLayer);
				}
					break;
				}
			}
		}
		
		// New base layer
		if (remoteTileSource != null)
			tileSource = remoteTileSource;

		if (tileSource != null)
		{
			// Set up the layer
			SphericalMercatorCoordSystem coordSys = new SphericalMercatorCoordSystem();
			baseLayer = new QuadImageTileLayer(mapControl,coordSys,tileSource);
			baseLayer.setSimultaneousFetches(8);

			// Cache directory for tiles
			if (remoteTileSource != null && cacheDirName != null)
			{
				File cacheDir = new File(getActivity().getCacheDir(),cacheDirName);
				cacheDir.mkdir();
				remoteTileSource.setCacheDir(cacheDir);
			}
				
			mapControl.addLayer(baseLayer);		
		}
	}
	
}
